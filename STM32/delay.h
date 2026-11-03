/**
  ******************************************************************************
  * @file    delay.h
  * @brief   Dinh thoi chinh xac su dung Timer 2 (1us tick) tren STM32F103
  ******************************************************************************
  */

#ifndef __DELAY_H
#define __DELAY_H

#include "stm32f10x.h"
#include <stdint.h>

void TIM2_Init(void);
void microDelay(uint16_t delay);
void delay_ms(uint32_t ms);

#endif /* __DELAY_H */
