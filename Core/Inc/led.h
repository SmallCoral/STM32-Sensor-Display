#ifndef __LED_H__
#define __LED_H__

#include "stm32f1xx_hal.h"

// 初始化
void LED_Init(void);

// 设置要显示的数字（0~199）
void DisplayNumber(uint16_t num);

// 在主循环中调用，用于刷新显示
void LED_Process(void);

#endif // __LED_H__
