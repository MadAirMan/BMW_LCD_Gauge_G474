//
// Created by Roman Smolyakov on 19.10.2024.
//

#ifndef BMW_CAN_MONITOR_GLOBAL_VARS_H
#define BMW_CAN_MONITOR_GLOBAL_VARS_H

#include <stdint-gcc.h>

#define GET_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
#define TIMEOUT_MS 500
#define TRANS_STR_SIZE 64

///Request Data ID
#define SEND_ID 0x6F1
///Response Data ID
#define ENGINE_DATA_ID 0x612
#define GEARBOX_DATA_ID 0x618

/// PID-коды для данных датчиков
#define PID_COOLANT_TEMP 0x05    // Температура охлаждающей жидкости
#define PID_ENGINE_OIL_TEMP 0xFD // Температура масла в двигателе
#define PID_GEAR_OIL_TEMP 0x01   // Температура масла в трансмиссии
#define PID_VOLTAGE 0x93         // Напряжение сети

#define MAX_RETRIES 5        // Максимальное число повторных попыток
#define TX_TIMEOUT 10           // Таймаут в мс для каждой попытки


//typedef enum { FALSE, TRUE } Bool;

// Pooling PIDs
extern const int16_t params[4];
#endif //BMW_CAN_MONITOR_GLOBAL_VARS_H
