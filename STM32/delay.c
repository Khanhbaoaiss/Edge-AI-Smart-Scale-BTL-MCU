/**
  ******************************************************************************
  * @file    delay.c
  * @brief   Cai dat cac ham dinh thoi microsecond va millisecond bang TIM2
  ******************************************************************************
  */

#include "delay.h"

void TIM2_Init(void)
{
  RCC->APB1ENR |= RCC_APB1ENR_TIM2EN; 
  TIM2->PSC = 71;            /* 72MHz / (71 + 1) = 1MHz -> dem 1 us */
  TIM2->ARR = 0xFFFF;
  TIM2->EGR = TIM_EGR_UG;    /* Ep nap ngay gia tri 71 vao active prescaler */
  TIM2->SR = 0;              /* Xoa co ngat */
  TIM2->CR1 |= TIM_CR1_CEN;  /* Bat counter */
}

void microDelay(uint16_t delay)
{
  TIM2->CNT = 0;
  while (TIM2->CNT < delay);
}

void delay_ms(uint32_t ms)
{
  while (ms--) {
    microDelay(1000);
  }
}
