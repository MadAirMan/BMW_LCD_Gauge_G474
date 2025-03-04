//
// Created by Roman Smolyakov on 04.03.2025.
//

#ifndef BMW_LCD_GAUGE_G474_DISPLAY_H
#define BMW_LCD_GAUGE_G474_DISPLAY_H

#include "stm32g4xx_hal.h"
#include "global_vars.h"

void Display_Init();
void Update_Coolant_Temperature_Display(int value_temp_digits);
void Update_Oil_Temperature_Display(int value_temp_digits);
void Update_Gear_Oil_Temperature_Display(int value_temp_digits);
void Update_Volt_Display(double value);
void display_UpdateElementInt(uint16_t parametr, int16_t value);
void display_UpdateElementFloat(uint16_t parametr, double value);

#endif //BMW_LCD_GAUGE_G474_DISPLAY_H
