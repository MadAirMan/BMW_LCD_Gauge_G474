//
// Created by Roman Smolyakov on 04.03.2025.
//

#include "obd2.h"
#include "NV3030B.h"
#include "global_vars.h"
#include "image.h"

////массив цифр для дисплея
const uint16_t* digits_num[11];

//////Инициализация дисплея
void Display_Init(void) {
    NV3030B_Init();
    digits_num[0] = digits_0;
    digits_num[1] = digits_1;
    digits_num[2] = digits_2;
    digits_num[3] = digits_3;
    digits_num[4] = digits_4;
    digits_num[5] = digits_5;
    digits_num[6] = digits_6;
    digits_num[7] = digits_7;
    digits_num[8] = digits_8;
    digits_num[9] = digits_9;
    digits_num[10] = digits_empty;
    HAL_GPIO_WritePin(BLK_GPIO_Port, BLK_Pin, GPIO_PIN_SET);
    NV3030B_FillScreen(NV3030B_BLACK);
    NV3030B_DrawBitmap(7, 2, coolant_temp, 66, 48, NV3030B_AMBER);
    NV3030B_DrawBitmap(0, 76, oil_temp, 79, 48, NV3030B_AMBER_DARK);
    NV3030B_DrawBitmap(5, 146, gear_temp, 61, 51, NV3030B_AMBER);
    NV3030B_DrawBitmap(7, 225, battery, 56, 38, NV3030B_AMBER_DARK);
    NV3030B_DrawImage(190, 22, 43, 30, celsium);
    NV3030B_DrawImage(190, 96, 43, 30, celsium);
    NV3030B_DrawImage(190, 166, 43, 30, celsium);
    NV3030B_DrawImage(205, 235, 28, 28, volt);
}

////Обновление температуры мотора
void Update_Coolant_Temperature_Display(int value_temp_digits) {
    if (value_temp_digits < 10) {
        NV3030B_DrawImage(79, 10, 36, 41, digits_num[10]);
        NV3030B_DrawImage(115, 10, 36, 41, digits_num[value_temp_digits]);
        NV3030B_DrawImage(151, 10, 36, 41, digits_num[10]);
    } else if (value_temp_digits < 100) {
        NV3030B_DrawImage(79, 10, 18, 41, digits_num[10]);
        NV3030B_DrawImage(97, 10, 36, 41, digits_num[value_temp_digits / 10]);
        NV3030B_DrawImage(133, 10, 36, 41, digits_num[value_temp_digits % 10]);
        NV3030B_DrawImage(169, 10, 18, 41, digits_num[10]);
    } else if (value_temp_digits < 1000) {
        NV3030B_DrawImage(79, 10, 36, 41, digits_num[value_temp_digits / 100]);
        NV3030B_DrawImage(115, 10, 36, 41, digits_num[(value_temp_digits % 100) / 10]);
        NV3030B_DrawImage(151, 10, 36, 41, digits_num[value_temp_digits % 10]);
    }
}

////Обновление температуры масла мотора
void Update_Oil_Temperature_Display(int value_temp_digits) {
    if (value_temp_digits < 10) {
        NV3030B_DrawImage(79, 80, 36, 41, digits_num[10]);
        NV3030B_DrawImage(115, 80, 36, 41, digits_num[value_temp_digits]);
        NV3030B_DrawImage(151, 80, 36, 41, digits_num[10]);
    } else if (value_temp_digits < 100) {
        NV3030B_DrawImage(79, 80, 18, 41, digits_num[10]);
        NV3030B_DrawImage(97, 80, 36, 41, digits_num[value_temp_digits / 10]);
        NV3030B_DrawImage(133, 80, 36, 41, digits_num[value_temp_digits % 10]);
        NV3030B_DrawImage(169, 80, 18, 41, digits_num[10]);
    } else if (value_temp_digits < 1000) {
        NV3030B_DrawImage(79, 80, 36, 41, digits_num[value_temp_digits / 100]);
        NV3030B_DrawImage(115, 80, 36, 41, digits_num[(value_temp_digits % 100) / 10]);
        NV3030B_DrawImage(151, 80, 36, 41, digits_num[value_temp_digits % 10]);
    }
}

////Обновление температуры коробки передач
void Update_Gear_Oil_Temperature_Display(int value_temp_digits) {
    if (value_temp_digits < 10) {
        NV3030B_DrawImage(79, 150, 36, 41, digits_num[10]);
        NV3030B_DrawImage(115, 150, 36, 41, digits_num[value_temp_digits]);
        NV3030B_DrawImage(151, 150, 36, 41, digits_num[10]);
    } else if (value_temp_digits < 100) {
        NV3030B_DrawImage(79, 150, 18, 41, digits_num[10]);
        NV3030B_DrawImage(97, 150, 36, 41, digits_num[value_temp_digits / 10]);
        NV3030B_DrawImage(133, 150, 36, 41, digits_num[value_temp_digits % 10]);
        NV3030B_DrawImage(169, 150, 18, 41, digits_num[10]);
    } else if (value_temp_digits < 1000) {
        NV3030B_DrawImage(79, 150, 36, 41, digits_num[value_temp_digits / 100]);
        NV3030B_DrawImage(115, 150, 36, 41, digits_num[(value_temp_digits % 100) / 10]);
        NV3030B_DrawImage(151, 150, 36, 41, digits_num[value_temp_digits % 10]);
    }
}

////Обновление напряжения
void Update_Volt_Display(double value) {
    int int_part = (int)value;
    int frac_part = (int)((value - int_part) * 10); // Adjust the multiplier for more precision
    if (value < 10) {
        NV3030B_DrawImage(79, 223, 18, 41, digits_num[10]);
        NV3030B_DrawImage(97, 223, 36, 41, digits_num[int_part]);
        NV3030B_DrawImage(133, 222, 7, 41, point);
        NV3030B_DrawImage(140, 223, 36, 41, digits_num[frac_part]);
        NV3030B_DrawImage(176, 223, 18, 41, digits_num[10]);
    } else if (value < 100) {
        NV3030B_DrawImage(79, 223, 36, 41, digits_num[int_part / 10]);
        NV3030B_DrawImage(115, 223, 36, 41, digits_num[(int_part % 10)]);
        NV3030B_DrawImage(151, 222, 7, 41, point);
        NV3030B_DrawImage(158, 223, 36, 41, digits_num[frac_part]);
    }
}

void display_UpdateElementInt(uint16_t can_id, int16_t value) {
    switch (can_id) {
        case PID_COOLANT_TEMP:
            Update_Coolant_Temperature_Display(value);
            break;
        case PID_ENGINE_OIL_TEMP:
            Update_Oil_Temperature_Display(value);
            break;
        case PID_GEAR_OIL_TEMP:
            Update_Gear_Oil_Temperature_Display(value);
            break;
        default:
            Update_Coolant_Temperature_Display(0);
            Update_Oil_Temperature_Display(0);
            Update_Gear_Oil_Temperature_Display(0);
    }
}

void display_UpdateElementFloat(uint16_t can_id, double value) {
    switch (can_id) {
        case PID_VOLTAGE:
            Update_Volt_Display(value);
            break;
        default:
            Update_Volt_Display(0);
    }
}