/**
  ******************************************************************************
  * @file    main.h
  * @brief   Tep tieu de trung tam cho he thong Can thong minh STM32
  ******************************************************************************
  */

#ifndef __MAIN_H
#define __MAIN_H

/* CMSIS Device standard header */
#include "stm32f10x.h"

/* Standard C libraries */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

/* Cac module trinh dieu khien ngoai vi */
#include "delay.h"
#include "i2c_soft.h"
#include "lcd1602.h"
#include "hx711.h"
#include "uart_esp32.h"

/* Bien toan cuc he thong */
extern volatile int32_t weight;
extern volatile int32_t last_sent_weight;

/* Bo dem hien thi LCD */
extern char lcd_line0[17];
extern char lcd_line1[17];
extern char prev_line0[17];
extern char prev_line1[17];

/* Nguyen mau ham he thong */
void SystemClock_Config(void);

#endif /* __MAIN_H */
