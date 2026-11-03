/**
  ******************************************************************************
  * @file    i2c_soft.c
  * @brief   Cai dat trinh dieu khien I2C Bit-Banging (PB6 - SCL, PB7 - SDA)
  ******************************************************************************
  */

#include "i2c_soft.h"

#define SCL_H() GPIOB->BSRR = (1 << 6)
#define SCL_L() GPIOB->BRR  = (1 << 6)
#define SDA_H() GPIOB->BSRR = (1 << 7)
#define SDA_L() GPIOB->BRR  = (1 << 7)
#define SDA_IN() ((GPIOB->IDR & (1 << 7)) ? 1 : 0)

void I2C_Soft_Init(void)
{
  RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;

  /* PB6, PB7: Output Open-Drain 50MHz (CNF=01, MODE=11 -> 0x7) */
  GPIOB->CRL &= ~(0xFF000000);
  GPIOB->CRL |=   0x77000000;

  SDA_H();
  SCL_H();
}

static void I2C_Delay(void)
{
  for (volatile int i = 0; i < 35; i++); /* Xung I2C ~100kHz */
}

void I2C_Start(void)
{
  SDA_H();
  SCL_H();
  I2C_Delay();
  SDA_L();
  I2C_Delay();
  SCL_L();
  I2C_Delay();
}

void I2C_Stop(void)
{
  SDA_L();
  I2C_Delay();
  SCL_H();
  I2C_Delay();
  SDA_H();
  I2C_Delay();
}

uint8_t I2C_WriteByte(uint8_t data)
{
  for (uint8_t i = 0; i < 8; i++)
  {
    if (data & 0x80) SDA_H();
    else SDA_L();
    data <<= 1;
    I2C_Delay();
    SCL_H();
    I2C_Delay();
    SCL_L();
    I2C_Delay();
  }

  /* Doc ACK */
  SDA_H();
  I2C_Delay();
  SCL_H();
  I2C_Delay();
  uint8_t ack = SDA_IN();
  SCL_L();
  I2C_Delay();
  return ack;
}
