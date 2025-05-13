#ifndef __LED_H
#define __LED_H

#include "stm32f1xx_hal.h"

// 显示行数（PB6–PB11 控制 LED 行，共6行）
#define LED_ROWS 6

// 用户应写入的显存（每个元素为一个行的列亮灭状态）
extern uint8_t vram[LED_ROWS];

/**
 * @brief 初始化 LED 显示所需的 GPIOB 引脚（PB0–PB15）
 *        PB0–PB5 为列控制输出，PB6–PB15 为行控制输出
 */
void LED_Init(void);

/**
 * @brief 启动 LED 刷新定时器中断（需要用户在 main.c 中初始化 TIM14）
 */
void LED_Start(void);

/**
 * @brief 定时器中断回调处理函数
 *        需要在 HAL_TIM_PeriodElapsedCallback 中调用本函数
 * @param htim HAL定时器句柄
 */
void LED_UpdateDisplay(TIM_HandleTypeDef *htim);

#endif // __LED_H
