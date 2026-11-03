/**
  ******************************************************************************
  * @file    uart_esp32.c
  * @brief   Cai dat trinh dieu khien USART1 va phan tich ban tin giao thuc ESP32-CAM
  ******************************************************************************
  */

#include "uart_esp32.h"
#include "main.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Bien giao tiep UART ESP32-CAM */
volatile char uart_rx_buf[UART_RX_BUF_SIZE];
volatile uint8_t uart_rx_idx = 0;
volatile uint8_t uart_data_ready = 0;
volatile uint8_t usart1_tx_busy = 0;
volatile uint8_t req_pending = 0;

char fruit_name[16] = "";
int32_t fruit_unit_price = 0;     /* Don gia VND/kg de tinh tien theo khoi luong */
int32_t fruit_price = 0;          /* Thanh tien VND sau khi tinh toan */
uint8_t fruit_detected = 0;

void USART1_Init(void)
{
  RCC->APB2ENR |= RCC_APB2ENR_USART1EN | RCC_APB2ENR_IOPAEN;

  /* PA9 (TX): Alternate Function Push-Pull 50MHz (CNF=10, MODE=11 -> 0x0B) */
  GPIOA->CRH &= ~(0x0F << 4);
  GPIOA->CRH |=  (0x0B << 4);

  /* PA10 (RX): Input Floating (CNF=01, MODE=00 -> 0x04) */
  GPIOA->CRH &= ~(0x0F << 8);
  GPIOA->CRH |=  (0x04 << 8);

  /* Baudrate 115200 @ 72MHz */
  USART1->BRR = 0x271;

  USART1->CR1 = USART_CR1_UE | USART_CR1_TE | USART_CR1_RE | USART_CR1_RXNEIE;

  NVIC_SetPriority(USART1_IRQn, 1);
  NVIC_EnableIRQ(USART1_IRQn);
}

void USART1_SendChar(char c)
{
  while (!(USART1->SR & USART_SR_TXE));
  USART1->DR = (uint8_t)c;
}

void USART1_SendString(const char *str)
{
  usart1_tx_busy = 1;
  while (*str)
  {
    USART1_SendChar(*str++);
  }
  usart1_tx_busy = 0;
}

void USART1_SendWeight(int32_t weight_val)
{
  char tx_buf[32];
  if (weight_val < 0) weight_val = 0;
  snprintf(tx_buf, sizeof(tx_buf), "W:%ld\r\n", (long)weight_val);
  USART1_SendString(tx_buf);
}

/* --- NGAT NHAN USART1 (DA SUA CHUA CHONG XOA BO DEM) --- */
void USART1_IRQHandler(void)
{
  if (USART1->SR & USART_SR_ORE)
  {
    (void)USART1->SR;
    (void)USART1->DR;
  }

  if (USART1->SR & USART_SR_RXNE)
  {
    uint8_t raw_b = (uint8_t)(USART1->DR & 0xFF);

    /* KHOA BO DEM: Goi RES dang cho main() xu ly -> bo qua MOI byte toi
       (ke ca \r \n) cho den khi main() reset uart_rx_idx = 0, uart_data_ready = 0 */
    if (uart_data_ready)
    {
      return;
    }

    /* Bo qua byte dau dong neu la byte nhi phan 0xAA hoac 0x55 */
    if (uart_rx_idx == 0 && (raw_b == 0xAA || raw_b == 0x55))
    {
      return;
    }

    char c = (char)raw_b;
    if (c == '\n' || c == '\r')
    {
      if (uart_rx_idx > 0)
      {
        uart_rx_buf[uart_rx_idx] = '\0';
        
        if (strncmp((char*)uart_rx_buf, "REQ", 3) == 0)
        {
          if (!usart1_tx_busy)
          {
            char req_buf[24];
            int32_t current_w = weight;
            if (current_w < 0) current_w = 0;
            snprintf(req_buf, sizeof(req_buf), "W:%ld\r\n", (long)current_w);
            char *p = req_buf;
            while (*p)
            {
              while (!(USART1->SR & USART_SR_TXE));
              USART1->DR = (uint8_t)(*p++);
            }
          }
          else
          {
            req_pending = 1;
          }
          uart_rx_idx = 0;
        }
        else if (strstr((char*)uart_rx_buf, "RES:") != NULL)
        {
          /* Co du lieu RES hop le: Bat co va GIU NGUYEN bo dem cho main doc */
          uart_data_ready = 1;
        }
        else
        {
          /* Neu la chuoi rac khong hop le thi moi xoa index */
          uart_rx_idx = 0;
        }
      }
    }
    else if (c >= 32 && c <= 126)
    {
      /* Neu co goi cu chua duoc main() doc thi tu dong reset de ghi goi moi */
      if (uart_data_ready == 0)
      {
        if (uart_rx_idx < UART_RX_BUF_SIZE - 1)
        {
          uart_rx_buf[uart_rx_idx++] = c;
        }
      }
    }
  }
}

/* ================= XU LY DU LIEU NHAN TU ESP32-CAM ================= */
void Parse_ESP32_Response(char *msg)
{
  char *start = strstr(msg, "RES:");
  if (!start) return;
  char *p = start + 4;
  char temp_name[16] = {0};
  int i = 0;

  while (*p && *p != ',' && i < 15)
  {
    temp_name[i++] = *p++;
  }
  temp_name[i] = '\0';

  if (strcasecmp(temp_name, "none") == 0 || temp_name[0] == '\0')
  {
    fruit_detected = 0;
    fruit_name[0] = '\0';
    fruit_unit_price = 0;
    fruit_price = 0;
    return;
  }

  fruit_detected = 1;
  if (strcasecmp(temp_name, "apple") == 0)
  {
    strcpy(fruit_name, "Apple");
    fruit_unit_price = PRICE_PER_KG_APPLE;
  }
  else if (strcasecmp(temp_name, "orange") == 0)
  {
    strcpy(fruit_name, "Orange");
    fruit_unit_price = PRICE_PER_KG_ORANGE;
  }
  else if (strcasecmp(temp_name, "pear") == 0)
  {
    strcpy(fruit_name, "Pear");
    fruit_unit_price = PRICE_PER_KG_PEAR;
  }
  else
  {
    strncpy(fruit_name, temp_name, sizeof(fruit_name) - 1);
    fruit_name[sizeof(fruit_name) - 1] = '\0';
    fruit_unit_price = 50000L;
  }

  /* Bo qua truong confidence */
  if (*p == ',') p++;
  while (*p && *p != ',') p++;

  /* Doc khoi luong */
  int32_t msg_weight_g = 0;
  if (*p == ',')
  {
    p++;
    msg_weight_g = (int32_t)atoi(p);
    while (*p && *p != ',') p++;
  }

  /* Doc thanh tien */
  int32_t parsed_price = 0;
  if (*p == ',')
  {
    p++;
    while (*p >= '0' && *p <= '9')
    {
      parsed_price = parsed_price * 10 + (*p - '0');
      p++;
    }
  }

  if (parsed_price > 0)
  {
    fruit_price = parsed_price;
    int32_t ref_w = (msg_weight_g > 0) ? msg_weight_g : weight;
    if (ref_w > 0)
    {
      fruit_unit_price = (parsed_price * 1000L + (ref_w / 2)) / ref_w;
    }
  }
  else if (weight > 0)
  {
    fruit_price = (weight * fruit_unit_price + 500) / 1000;
  }
}
