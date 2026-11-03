/**
  ******************************************************************************
  * @file    lcd1602.h
  * @brief   Trinh dieu khien LCD 1602 giao tiep qua I2C Backpack (PCF8574)
  ******************************************************************************
  */

#ifndef __LCD1602_H
#define __LCD1602_H

#include "stm32f10x.h"
#include <stdint.h>

#define LCD_ADDR          0x4E       /* 0x27 << 1 (Dia chi I2C PCF8574) */

void LCD_Init(void);
void PCF8574_Write(uint8_t data);
void LCD_WriteNibble(uint8_t nibble, uint8_t rs);
void LCD_Send_Cmd(uint8_t cmd);
void LCD_Send_Data(uint8_t data);
void LCD_SetCursor(uint8_t row, uint8_t col);
void LCD_Print(const char *str);
void LCD_Clear(void);

#endif /* __LCD1602_H */
