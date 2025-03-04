/*
 * obd2.c
 *
 *  Created on: Feb 8, 2024
 *      Author: aivs
 */

/* Private includes ----------------------------------------------------------*/
#include "obd2.h"
#include "string.h"
#include <stdio.h>
#include <stdbool.h>
#include "global_vars.h"
#include "math.h"
#include "display.h"
/* Private variables ---------------------------------------------------------*/
/// CAN Data Transmit Setup
extern FDCAN_HandleTypeDef hfdcan1; // CAN Data Transmit Setup
///переменная для инициализации UART
extern UART_HandleTypeDef huart2;

extern bool OBD2_NEED_NEXT_REQUEST;

static uint32_t last_request_time = 0;
static uint8_t num = 0;

// Буфер для логов
char log_str[128];

/* Private function prototypes -----------------------------------------------*/
void obd2_SendFrame(uint16_t address, uint8_t byte0, uint8_t byte1, uint8_t byte2, uint8_t byte3, uint8_t byte4,
                    uint8_t byte5, uint8_t byte6, uint8_t byte7);

/* Private user code ---------------------------------------------------------*/
void obd2_NextRequest() {
    num = (num == GET_SIZE(params)) ? 0 : num;
    switch (params[num]) {
        case PID_COOLANT_TEMP :
            obd2_SendFrame(SEND_ID, 0x12, 0x04, 0x2C, 0x10, 0x00, params[num++], 0x00, 0x00);
            break;
        case PID_ENGINE_OIL_TEMP:
            obd2_SendFrame(SEND_ID, 0x12, 0x04, 0x2C, 0x10, 0x00, params[num++], 0x00, 0x00);
            break;
        case PID_GEAR_OIL_TEMP:
            obd2_SendFrame(SEND_ID, 0x18, 0x02, 0x21, params[num++], 0x00, 0x00, 0x00, 0x00);
            break;
        case PID_VOLTAGE:
            obd2_SendFrame(SEND_ID, 0x12, 0x04, 0x2C, 0x10, 0x00, params[num++], 0x00, 0x00);
            break;
    }
}

//// Функция для обработки полученных данных с целочисленными значениями
void valueHandlerInt(uint16_t params, uint16_t value) {
    int len = snprintf(log_str, sizeof(log_str), "VALUE - %i\n", value);
    if (len < 0 || len >= (int)sizeof(log_str)) {
        HAL_UART_Transmit(&huart2, (uint8_t *)"Format error\n", 13, 100);
    } else {
        HAL_UART_Transmit(&huart2, (uint8_t *)log_str, len, 100);
    }
    OBD2_NEED_NEXT_REQUEST = true;
    display_UpdateElementInt(params, value);
}

//// Функция для обработки полученных данных с целочисленными значениями
void valueHandlerFloat(uint16_t params, double value) {
    int len = snprintf(log_str, sizeof(log_str), "VALUE - %.2f\n", value);
    if (len < 0 || len >= (int)sizeof(log_str)) {
        HAL_UART_Transmit(&huart2, (uint8_t *)"Format error\n", 13, 100);
    } else {
        HAL_UART_Transmit(&huart2, (uint8_t *)log_str, len, 100);
    }
    OBD2_NEED_NEXT_REQUEST = true;
    display_UpdateElementFloat(params, value);
}


void obd2_Handler(uint8_t RxData[]) {
    uint8_t data1 = RxData[4];
	uint8_t data2 = RxData[5];

    int16_t value;
    double value_float;

    switch (params[num-1]) {
        case PID_COOLANT_TEMP:
            value = data2 - 40;
            break;
        case PID_ENGINE_OIL_TEMP:
            value = (data1 << 8 | data2) / 10 - 60;
            break;
        case PID_GEAR_OIL_TEMP:
            value = data1 - 40;;
            break;
        case PID_VOLTAGE:
            value_float = ceil((data1 << 8 | data2) * 0.00245 * 10) / 10;
            break;
        default:
            value = data1;
    }
    if (params[num-1] != PID_VOLTAGE){
        valueHandlerInt(params[num-1], value);
    } else {
        valueHandlerFloat(params[num-1], value_float);
    }
}



void obd2_SendFrame(uint16_t address, uint8_t byte0, uint8_t byte1, uint8_t byte2, uint8_t byte3, uint8_t byte4,
                    uint8_t byte5, uint8_t byte6, uint8_t byte7) {

	// Настройка CAN перед отправкой
    FDCAN_TxHeaderTypeDef TxHeader;
    uint8_t TxData[8];

    TxHeader.Identifier = address;
    TxHeader.IdType = FDCAN_STANDARD_ID;
    TxHeader.TxFrameType = FDCAN_DATA_FRAME;
    TxHeader.DataLength = FDCAN_DLC_BYTES_8;
    TxHeader.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
    TxHeader.BitRateSwitch = FDCAN_BRS_OFF;
    TxHeader.FDFormat = FDCAN_CLASSIC_CAN;
    TxHeader.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
    TxHeader.MessageMarker = 0;

    TxData[0] = byte0;
    TxData[1] = byte1;
    TxData[2] = byte2;
    TxData[3] = byte3;
    TxData[4] = byte4;
    TxData[5] = byte5;
    TxData[6] = byte6;
    TxData[7] = byte7;

// Попытки отправить сообщение
    for (int attempt = 1; attempt <= MAX_RETRIES; attempt++) {
        // Ожидание свободного места в TX FIFO
        while (HAL_FDCAN_GetTxFifoFreeLevel(&hfdcan1) == 0) {
            sprintf(log_str, "CAN Mailbox Busy (Attempt %d)\n", attempt);
            HAL_UART_Transmit(&huart2, (uint8_t*)log_str, strlen(log_str), HAL_MAX_DELAY);
        	HAL_Delay(5); // Ждем освобождения FIFO
        }

        // Проверяем состояние CAN перед отправкой
        if (HAL_FDCAN_GetState(&hfdcan1) != HAL_FDCAN_STATE_BUSY) {
            HAL_FDCAN_Start(&hfdcan1);
        }

        // Попытка отправить сообщение
        if (HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &TxHeader, TxData) == HAL_OK) {
        	// Если успех – логируем
            snprintf(log_str, sizeof(log_str), "%08lu CanTx: %lX [%ld] ", HAL_GetTick(), TxHeader.Identifier, TxHeader.DataLength);
            for (uint32_t i = 0; i < TxHeader.DataLength; i++) {
                char buffer[4];
                snprintf(buffer, sizeof(buffer), "%02X", TxData[i]); // Преобразуем значение в строку
                strcat(log_str, buffer); // Добавляем значение к основной строке
                if (i < 7) { // Добавляем запятую между значениями, кроме последнего
                    strcat(log_str, " ");
                }
            }
            strcat(log_str, "\n");
            HAL_UART_Transmit(&huart2, (uint8_t *) log_str, strlen(log_str), 100);

            last_request_time = HAL_GetTick();
            return;  // Выход из функции после успешной отправки
        } else {
            sprintf(log_str, "CAN TX Error (Attempt %d, ID: 0x%X)\n", attempt, address);
            HAL_UART_Transmit(&huart2, (uint8_t*)log_str, strlen(log_str), HAL_MAX_DELAY);
        }

        // Проверка ошибок
        uint32_t error = HAL_FDCAN_GetError(&hfdcan1);
        if (error != HAL_FDCAN_ERROR_NONE) {
            sprintf(log_str, "CAN Error: 0x%08lX\n", error);
            HAL_UART_Transmit(&huart2, (uint8_t*)log_str, strlen(log_str), HAL_MAX_DELAY);
        }

        // Задержка перед следующей попыткой
        HAL_Delay(TX_TIMEOUT);
    }

    // Если после всех попыток отправка не удалась
    sprintf(log_str, "CAN TX FAILED after %d attempts (ID: 0x%X)\n", MAX_RETRIES, address);
    HAL_UART_Transmit(&huart2, (uint8_t*)log_str, strlen(log_str), HAL_MAX_DELAY);
}

uint32_t obd2_getLastRequestTime() {
    return last_request_time;
}
