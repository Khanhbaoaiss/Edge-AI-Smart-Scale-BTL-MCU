/**
  ******************************************************************************
  * @file    hx711.c
  * @brief   Cai dat doc ADC 24-bit, loc nhieu 12 mau va hieu chuan Tare cho HX711
  ******************************************************************************
  */

#include "hx711.h"
#include "delay.h"

float knownOriginal = 194.0f;     
float knownHX711    = 81515.0f;   /* He so can chuan xac cho vat mau 194g */
int32_t tare        = 0;          /* Signed int32_t tranh loi offset am */

void HX711_Init(void)
{
  RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;

  /* PB8 (DT): Input Floating (CNF=01, MODE=00 -> 0x04) */
  GPIOB->CRH &= ~(0x0F << 0);
  GPIOB->CRH |=  (0x04 << 0);

  /* PB9 (SCK): Output Push-Pull 2MHz (CNF=00, MODE=10 -> 0x02) */
  GPIOB->CRH &= ~(0x0F << 4);
  GPIOB->CRH |=  (0x02 << 4);

  GPIOB->BRR = SCK_PIN; 
}

int32_t HX711_ReadRaw(void)
{
  uint32_t data = 0;
  uint32_t timeout = 500000;

  while ((GPIOB->IDR & DT_PIN) && (--timeout > 0));
  if (timeout == 0) return 0;

  for (int8_t i = 0; i < 24; i++)
  {
    GPIOB->BSRR = SCK_PIN; 
    microDelay(1);
    data = data << 1;
    GPIOB->BRR = SCK_PIN;  
    microDelay(1);

    if (GPIOB->IDR & DT_PIN)
    {
      data++;
    }
  }

  GPIOB->BSRR = SCK_PIN;
  microDelay(1);
  GPIOB->BRR = SCK_PIN;
  microDelay(1);

  if (data & 0x00800000)
  {
    data |= 0xFF000000;
  }

  return (int32_t)data;
}

int32_t HX711_GetWeight(void)
{
  int64_t total = 0;
  int32_t samples = 12;
  int32_t max_val = -2147483647;
  int32_t min_val = 2147483647;
  int32_t raw_val;
  int32_t average;
  float calculated_weight;

  for (int32_t i = 0; i < samples; i++)
  {
    raw_val = HX711_ReadRaw();
    if (raw_val > max_val) max_val = raw_val;
    if (raw_val < min_val) min_val = raw_val;
    total += raw_val;
  }

  total = total - max_val - min_val;
  average = (int32_t)(total / (samples - 2));

  calculated_weight = (float)(average - tare) * (knownOriginal / knownHX711);

  if (calculated_weight >= 0)
    return (int32_t)(calculated_weight + 0.5f);
  else
    return (int32_t)(calculated_weight - 0.5f);
}

void HX711_Tare(uint8_t samples)
{
  int64_t tare_sum = 0;

  /* Reset HX711 */
  GPIOB->BSRR = SCK_PIN;
  delay_ms(10);
  GPIOB->BRR = SCK_PIN;
  delay_ms(500);

  if (samples == 0) samples = 10;
  for (uint8_t i = 0; i < samples; i++)
  {
    tare_sum += HX711_ReadRaw();
    delay_ms(20);
  }
  tare = (int32_t)(tare_sum / samples);
}
