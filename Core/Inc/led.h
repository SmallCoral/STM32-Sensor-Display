#ifndef __LED_H
#define __LED_H

#include "stm32f1xx_hal.h"  // 包含 STM32F1 系列的 HAL 库头文件


// 函数声明
void ClearAllSegments(void);
void DisplayNumber(uint16_t num);
void Set_LevelBar(uint32_t temp);
void RefreshDisplay(void);
void LED_Init(void);
void LED_Process(void);
void LED_Show(void);
void DisplayHotTemp();
uint32_t ConvertToFahrenheit(uint32_t celsius);
void ntc();

extern uint8_t display_celsius;  // 添加这行

#endif // __LED_H
