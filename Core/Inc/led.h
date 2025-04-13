#ifndef __LED_H
#define __LED_H

#include "stm32f1xx_hal.h"

// 数码管显示函数：输入范围为 0 ~ 199
void DisplayNumber(uint16_t num);

// 初始化函数（如需 GPIO 初始化，可选实现）
void LED_Init(void);

// 清除所有段位（可外部调用强制清除）
void ClearAllSegments(void);

#endif /* __LED_H */
