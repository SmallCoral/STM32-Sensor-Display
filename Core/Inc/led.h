#ifndef __LED_H
#define __LED_H

#include "stm32f1xx_hal.h"

// 声明外部定时器句柄
extern TIM_HandleTypeDef htim2;

void LED_Init(void);
void DisplayNumber(uint16_t num);

#endif