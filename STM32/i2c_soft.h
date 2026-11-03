/**
  ******************************************************************************
  * @file    i2c_soft.h
  * @brief   Giao tiep I2C bang phuong phap Bit-Banging tren GPIO (PB6 SCL, PB7 SDA)
  ******************************************************************************
  */

#ifndef __I2C_SOFT_H
#define __I2C_SOFT_H

#include "stm32f10x.h"
#include <stdint.h>

void I2C_Soft_Init(void);
void I2C_Start(void);
void I2C_Stop(void);
uint8_t I2C_WriteByte(uint8_t data);

#endif /* __I2C_SOFT_H */
