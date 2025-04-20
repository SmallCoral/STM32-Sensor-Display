#include "key.h"

// 检测按键状态（PC13）
uint8_t KEY_GetState(void) {
    if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13) == GPIO_PIN_RESET) { // 按键按下（假设低电平有效）
        HAL_Delay(20); // 简单消抖
        if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13) == GPIO_PIN_RESET) {
            return 1; // 按键确实按下
        }
    }
    return 0; // 按键未按下
}
