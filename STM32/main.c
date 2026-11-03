/**
  ******************************************************************************
  * @file    main.c
  * @brief   Chuong trinh dieu khien chinh he thong Can thong minh STM32F103
  *          - HX711: Can khoi luong
  *          - LCD 1602 (I2C): Hien thi khoi luong va thanh tien
  *          - USART1: Giao tiep voi ESP32-CAM AI Nhan dien trai cay
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private variables ---------------------------------------------------------*/
volatile int32_t weight = 0;      
volatile int32_t last_sent_weight = 0; /* Bien toan cuc xem tren Watch 1 */

/* Bo dem man hinh LCD chong tran (luon co dinh 16 ky tu + null) */
char lcd_line0[17];
char lcd_line1[17];
char prev_line0[17] = "";
char prev_line1[17] = "";

/* ================= SYSTEM CLOCK ================= */
void SystemClock_Config(void)
{
  RCC->CR |= RCC_CR_HSEON;
  while (!(RCC->CR & RCC_CR_HSERDY));

  FLASH->ACR = FLASH_ACR_PRFTBE | FLASH_ACR_LATENCY_2;

  RCC->CFGR &= ~(RCC_CFGR_PLLSRC | RCC_CFGR_PLLMULL);
  RCC->CFGR |= (RCC_CFGR_PLLSRC | RCC_CFGR_PLLMULL9);

  RCC->CR |= RCC_CR_PLLON;
  while (!(RCC->CR & RCC_CR_PLLRDY));

  RCC->CFGR &= ~(RCC_CFGR_HPRE | RCC_CFGR_PPRE1 | RCC_CFGR_PPRE2);
  RCC->CFGR |= RCC_CFGR_PPRE1_DIV2;

  RCC->CFGR &= ~RCC_CFGR_SW;
  RCC->CFGR |= RCC_CFGR_SW_PLL;
  while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);

  /* Cap clock cho Port D du phong */
  RCC->APB2ENR |= RCC_APB2ENR_IOPDEN;
}

/* ================= MAIN PROGRAM ================= */
int main(void)
{
  int32_t prev_weight = 0;
  uint32_t stable_timer = 0;
  uint32_t send_tick = 0;

  /* 1. Khoi tao he thong va ngoai vi */
  SystemClock_Config();
  TIM2_Init();
  I2C_Soft_Init();
  USART1_Init();
  HX711_Init();

  /* 2. Man hinh chao va hieu chuan ban dau */
  LCD_Init();
  LCD_SetCursor(0, 0);
  LCD_Print("  SMART SCALE   ");
  LCD_SetCursor(1, 0);
  LCD_Print(" Calibrating... ");

  /* 3. Reset va Tare lay diem 0 cho HX711 (lay 10 mau) */
  HX711_Tare(10);

  /* 4. Thong bao ngay cho ESP32-CAM biet STM32 da tare xong va khoi luong = 0g */
  USART1_SendString("W:0\r\n");

  /* 5. Khoi tao man hinh can mac dinh */
  LCD_Clear();
  LCD_SetCursor(0, 0);
  LCD_Print("WEIGHT VALUE:   ");
  strncpy(prev_line0, "WEIGHT VALUE:   ", 16);
  prev_line0[16] = '\0';

  /* 6. Vong lap chinh */
  while (1)
  {
    weight = HX711_GetWeight();
    if (weight < 0) weight = 0;

    /* Kiem tra co yeu cau REQ can phan hoi ngay khong */
    if (req_pending)
    {
      req_pending = 0;
      USART1_SendWeight(weight);
    }

    /* 6.1 Kiem tra co du lieu phan hoi tu ESP32-CAM qua USART1 khong */
    if (uart_data_ready)
    {
      char local_rx_copy[UART_RX_BUF_SIZE];
      strncpy(local_rx_copy, (const char*)uart_rx_buf, UART_RX_BUF_SIZE - 1);
      local_rx_copy[UART_RX_BUF_SIZE - 1] = '\0';
      
      /* Mo khoa bo dem de don goi tiep theo */
      uart_rx_idx = 0;
      uart_data_ready = 0;
      
      Parse_ESP32_Response(local_rx_copy);
    }

    /* 6.2 Xu ly khi can trong (< 15g) */
    if (weight < 15)
    {
      if (last_sent_weight > 0)
      {
        USART1_SendString("W:0\r\n");
        last_sent_weight = 0;
        fruit_detected = 0;
        fruit_name[0] = '\0';
        fruit_unit_price = 0;
        fruit_price = 0;
        stable_timer = 0;
        send_tick = 0;
      }

      snprintf(lcd_line0, sizeof(lcd_line0), "%-16s", "WEIGHT VALUE:   ");
      snprintf(lcd_line1, sizeof(lcd_line1), "      0 g       ");
    }
    /* 6.3 Xu ly khi co vat tren can (>= 15g) */
    else
    {
      int32_t sent_diff = weight - last_sent_weight;
      if (sent_diff < 0) sent_diff = -sent_diff;

      int32_t step_diff = weight - prev_weight;
      if (step_diff < 0) step_diff = -step_diff;

      /* Chi tang bo dem on dinh khi khoi luong thay doi >= 5g VA dao dong giua 2 chu ky <= 3g */
      if (sent_diff >= 5)
      {
        if (step_diff <= 3)
        {
          stable_timer += 100;
          if (stable_timer >= 300)
          {
            USART1_SendWeight(weight);
            last_sent_weight = weight;
            stable_timer = 0;
            send_tick = 0;
            fruit_detected = 0;
          }
        }
        else
        {
          stable_timer = 0;
        }
      }
      else
      {
        stable_timer = 0;
        if (!fruit_detected && (++send_tick >= 25))
        {
          USART1_SendWeight(weight);
          send_tick = 0;
        }
      }

      /* Cap nhat thanh tien VND theo khoi luong thuc te tren can */
      if (fruit_detected && fruit_unit_price > 0)
      {
        fruit_price = (weight * fruit_unit_price + 500) / 1000;
      }

      /* Hien thi LCD */
      if (fruit_detected)
      {
        snprintf(lcd_line0, sizeof(lcd_line0), "%-10s %4ldg", fruit_name, (long)weight);
        snprintf(lcd_line1, sizeof(lcd_line1), "Tien:%7ld VND", (long)fruit_price);
      }
      else
      {
        snprintf(lcd_line0, sizeof(lcd_line0), "%-16s", "WEIGHT VALUE:   ");
        snprintf(lcd_line1, sizeof(lcd_line1), "   %4ld g       ", (long)weight);
      }
    }

    prev_weight = weight;

    /* 6.4 CHI CAP NHAT LCD KHI CHU THAY DOI DE CHONG CHOP NHAY VA CHONG TRAN */
    if (strcmp(lcd_line0, prev_line0) != 0)
    {
      LCD_SetCursor(0, 0);
      LCD_Print(lcd_line0);
      strncpy(prev_line0, lcd_line0, 16);
      prev_line0[16] = '\0';
    }

    if (strcmp(lcd_line1, prev_line1) != 0)
    {
      LCD_SetCursor(1, 0);
      LCD_Print(lcd_line1);
      strncpy(prev_line1, lcd_line1, 16);
      prev_line1[16] = '\0';
    }

    delay_ms(100);
  }
}
