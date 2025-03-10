/*
 * obd2.c
 *
 *  Created on: Feb 8, 2024
 *      Author: aivs
 */

#ifndef INC_OBD2_C_
#define INC_OBD2_C_

#include "stm32g4xx_hal.h"
#include "global_vars.h"

void obd2_Handler(uint8_t RxData[]);
void obd2_NextRequest();
uint32_t obd2_getLastRequestTime();

#endif /* INC_OBD2_C_ */
