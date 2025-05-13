#include "main.h"
#include <string.h>

#define ROWS 6  // 共6行（LED1–LED6）
uint8_t vram[ROWS];               // 用户写入的显存
static uint8_t vram_real[ROWS];  // 实际刷新的显存
static uint8_t current_row = 0;

// 需要在main中定义外部定时器句柄
extern TIM_HandleTypeDef htim14;

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM2) {
        if (current_row == 0) {
            memcpy(vram_real, vram, sizeof(vram));
        }

        // // 关闭所有行（PB6-PB15），防止重影
        // for (uint8_t i = 6; i <= 15; i++) {
        //     HAL_GPIO_WritePin(GPIOB, (1 << i), GPIO_PIN_RESET);
        // }

        // 刷新当前行的数据（设置PB0-PB5为当前行要显示的列状态）
        for (uint8_t i = 0; i < 6; i++) {
            HAL_GPIO_WritePin(GPIOB, (1 << i), (vram_real[current_row] & (1 << i)) ? GPIO_PIN_SET : GPIO_PIN_RESET);
        }

        // 点亮当前行（共阴极，接GND → 输出为0 → GPIO置为RESET）
        // 例如 current_row = 0 → PB6
        HAL_GPIO_WritePin(GPIOB, (1 << (current_row + 6)), GPIO_PIN_SET);

        // 下一行
        current_row = (current_row + 1) % ROWS;
    }
}
