// led_display.h

#ifndef LED_DISPLAY_H
#define LED_DISPLAY_H

#include "stm32f1xx_hal.h"

// 函数声明
void LED_Display_Init(void);
void LED_Display_Update(void);
void LED_Display_SetBuffer(uint8_t col, uint16_t pattern);
void LED_Display_SetNumber(uint32_t number);

// 全局变量声明
extern volatile uint16_t display_buffer[6];
extern volatile uint8_t currentColumn;

#endif // LED_DISPLAY_H
