/**
  ******************************************************************************
  * @file    hx711.h
  * @brief   Trinh dieu khien cam bien Load Cell qua IC ADC 24-bit HX711
  ******************************************************************************
  */

#ifndef __HX711_H
#define __HX711_H

#include "stm32f10x.h"
#include <stdint.h>

#define DT_PIN            (1 << 8)   /* PB8: Data HX711 */
#define SCK_PIN           (1 << 9)   /* PB9: Clock HX711 */

extern float knownOriginal;     
extern float knownHX711;             /* He so can chuan xac cho vat mau 194g */
extern int32_t tare;                 /* Signed int32_t tranh loi offset am */

void HX711_Init(void);
int32_t HX711_ReadRaw(void);
int32_t HX711_GetWeight(void);
void HX711_Tare(uint8_t samples);

/* Giu lai ten ham cu de hoan toan tuong thich code goc */
#define getHX711() HX711_ReadRaw()
#define weigh()    HX711_GetWeight()

#endif /* __HX711_H */
