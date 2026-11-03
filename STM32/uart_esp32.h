/**
  ******************************************************************************
  * @file    uart_esp32.h
  * @brief   Giao tiep UART giua STM32 va module AI ESP32-CAM (USART1 115200bps)
  ******************************************************************************
  */

#ifndef __UART_ESP32_H
#define __UART_ESP32_H

#include "stm32f10x.h"
#include <stdint.h>

#define UART_RX_BUF_SIZE     64

/* Don gia mac dinh (VND / kg) du phong khi ESP32 chua gui gia */
#define PRICE_PER_KG_APPLE   60000L
#define PRICE_PER_KG_ORANGE  40000L
#define PRICE_PER_KG_PEAR    70000L

/* Bien giao tiep UART ESP32-CAM */
extern volatile char uart_rx_buf[UART_RX_BUF_SIZE];
extern volatile uint8_t uart_rx_idx;
extern volatile uint8_t uart_data_ready;
extern volatile uint8_t usart1_tx_busy;
extern volatile uint8_t req_pending;

/* Thong tin hoa qua nhan dien va tinh tien */
extern char fruit_name[16];
extern int32_t fruit_unit_price;     /* Don gia VND/kg */
extern int32_t fruit_price;          /* Thanh tien VND */
extern uint8_t fruit_detected;

/* Nguyen mau ham */
void USART1_Init(void);
void USART1_SendChar(char c);
void USART1_SendString(const char *str);
void USART1_SendWeight(int32_t weight_val);
void Parse_ESP32_Response(char *msg);

#endif /* __UART_ESP32_H */
