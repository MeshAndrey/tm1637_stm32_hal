/*
	Works correct with frequency <= 32 MHz	
*/
#ifndef TM1637_STM32_HAL_H
#define TM1637_STM32_HAL_H

#include "stm32f0xx_hal.h"

void tm1637Init(void);
void tm1637DisplayDecimal(uint32_t v, uint8_t displaySeparator);
void tm1637SetBrightness(uint8_t brightness);

#endif
