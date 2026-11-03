/**
  ******************************************************************************
  * @file    lcd1602.c
  * @brief   Cai dat cac lenh khoi tao va giao tiep LCD 1602 qua chip PCF8574
  ******************************************************************************
  */

#include "lcd1602.h"
#include "i2c_soft.h"
#include "delay.h"

void PCF8574_Write(uint8_t data)
{
  I2C_Start();
  I2C_WriteByte(LCD_ADDR);
  I2C_WriteByte(data);
  I2C_Stop();
}

void LCD_WriteNibble(uint8_t nibble, uint8_t rs)
{
  uint8_t data = (nibble & 0xF0) | (rs ? 0x01 : 0x00) | 0x08; /* Backlight = 1 */

  PCF8574_Write(data | 0x04);  /* En = 1 */
  microDelay(2);
  PCF8574_Write(data & ~0x04); /* En = 0 */
  microDelay(40);
}

void LCD_Send_Cmd(uint8_t cmd)
{
  LCD_WriteNibble(cmd & 0xF0, 0);
  LCD_WriteNibble((cmd << 4) & 0xF0, 0);

  if (cmd == 0x01 || cmd == 0x02) {
    delay_ms(3); /* Lenh Xoa can > 1.52ms */
  } else {
    microDelay(40);
  }
}

void LCD_Send_Data(uint8_t data)
{
  LCD_WriteNibble(data & 0xF0, 1);
  LCD_WriteNibble((data << 4) & 0xF0, 1);
  microDelay(40);
}

void LCD_Init(void)
{
  delay_ms(50); /* Doi dien ap LCD on dinh > 40ms */

  /* Chu trinh ep che do 4-bit chuan Hitachi HD44780 */
  LCD_WriteNibble(0x30, 0);
  delay_ms(5);
  LCD_WriteNibble(0x30, 0);
  delay_ms(1);
  LCD_WriteNibble(0x30, 0);
  delay_ms(1);

  /* Chi gui duy nhat 1 nua byte cao 0x20 de vao 4-bit */
  LCD_WriteNibble(0x20, 0);
  delay_ms(1);

  /* Tu day moi gui du 2 nibble */
  LCD_Send_Cmd(0x28); /* 2 dong, font 5x8 */
  delay_ms(1);
  LCD_Send_Cmd(0x08); /* Tat hien thi */
  delay_ms(1);
  LCD_Send_Cmd(0x01); /* Xoa toan bo man hinh */
  delay_ms(3);
  LCD_Send_Cmd(0x06); /* Tu dong tang con tro, khong cuon man hinh */
  delay_ms(1);
  LCD_Send_Cmd(0x0C); /* Bat hien thi, tat con tro nhap nhay */
  delay_ms(2);
}

void LCD_SetCursor(uint8_t row, uint8_t col)
{
  uint8_t address = (row == 0) ? (0x80 + (col & 0x0F)) : (0xC0 + (col & 0x0F));
  LCD_Send_Cmd(address);
}

void LCD_Print(const char *str)
{
  while (*str)
  {
    unsigned char c = (unsigned char)*str++;
    if (c >= 0x20 && c <= 0x7E)
    {
      LCD_Send_Data(c);
    }
    else if (c == 0xA0)
    {
      LCD_Send_Data(0x20);
    }
  }
}

void LCD_Clear(void)
{
  LCD_Send_Cmd(0x01);
  delay_ms(3);
}
