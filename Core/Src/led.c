#include "led.h"

// 7段数码管段码表
const uint8_t segment_table[10] = {
    0b1111110, // 0
    0b0110000, // 1
    0b1101101, // 2
    0b1111001, // 3
    0b0110011, // 4
    0b1011011, // 5
    0b1011111, // 6
    0b1110000, // 7
    0b1111111, // 8
    0b1111011  // 9
};

// 引脚定义
#define DIGIT_LEFT_PIN   GPIO_PIN_0
#define DIGIT_MID_PIN    GPIO_PIN_1
#define DIGIT_RIGHT_PIN  GPIO_PIN_2

#define SEG_A_PIN    GPIO_PIN_6
#define SEG_B_PIN    GPIO_PIN_7
#define SEG_C_PIN    GPIO_PIN_8
#define SEG_D_PIN    GPIO_PIN_9
#define SEG_E_PIN    GPIO_PIN_10
#define SEG_F_PIN    GPIO_PIN_11
#define SEG_G_PIN    GPIO_PIN_12

// 显示状态变量
static uint16_t current_number = 0;
static uint8_t current_digit = 0;
static uint8_t digits[3] = {0};

// 初始化LED
void LED_Init(void) {
    // 初始化GPIO
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    // 位选引脚
    GPIO_InitStruct.Pin = DIGIT_LEFT_PIN | DIGIT_MID_PIN | DIGIT_RIGHT_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    // 段选引脚
    GPIO_InitStruct.Pin = SEG_A_PIN | SEG_B_PIN | SEG_C_PIN | SEG_D_PIN |
                         SEG_E_PIN | SEG_F_PIN | SEG_G_PIN;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    // 初始状态关闭所有显示
    HAL_GPIO_WritePin(GPIOB, DIGIT_LEFT_PIN | DIGIT_MID_PIN | DIGIT_RIGHT_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOB, SEG_A_PIN | SEG_B_PIN | SEG_C_PIN | SEG_D_PIN |
                      SEG_E_PIN | SEG_F_PIN | SEG_G_PIN, GPIO_PIN_RESET);
}

// 设置要显示的数字
void DisplayNumber(uint16_t num) {
    if(num > 199) num = 199;
    current_number = num;

    digits[0] = num / 100;          // 百位
    digits[1] = (num % 100) / 10;   // 十位
    digits[2] = num % 10;           // 个位
}

// 在定时器中断回调中调用此函数
void UpdateDisplay(void) {
    // 先关闭所有位选
    HAL_GPIO_WritePin(GPIOB, DIGIT_LEFT_PIN | DIGIT_MID_PIN | DIGIT_RIGHT_PIN, GPIO_PIN_SET);

    // 根据当前位选择要显示的数字
    uint8_t digit_to_show = 0;
    switch(current_digit) {
        case 0:  // 百位
            if(current_number >= 100) {
                digit_to_show = digits[0];
                HAL_GPIO_WritePin(GPIOB, DIGIT_RIGHT_PIN, GPIO_PIN_RESET);
            }
            break;
        case 1:  // 十位
            if(current_number >= 10) {
                digit_to_show = digits[1];
                HAL_GPIO_WritePin(GPIOB, DIGIT_LEFT_PIN, GPIO_PIN_RESET);
            }
            break;
        case 2:  // 个位
            digit_to_show = digits[2];
            HAL_GPIO_WritePin(GPIOB, DIGIT_MID_PIN, GPIO_PIN_RESET);
            break;
    }

    // 设置段选
    uint8_t seg_pattern = segment_table[digit_to_show];
    HAL_GPIO_WritePin(GPIOB, SEG_A_PIN, (seg_pattern & 0b1000000) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOB, SEG_B_PIN, (seg_pattern & 0b0100000) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOB, SEG_C_PIN, (seg_pattern & 0b0010000) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOB, SEG_D_PIN, (seg_pattern & 0b0001000) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOB, SEG_E_PIN, (seg_pattern & 0b0000100) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOB, SEG_F_PIN, (seg_pattern & 0b0000010) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOB, SEG_G_PIN, (seg_pattern & 0b0000001) ? GPIO_PIN_SET : GPIO_PIN_RESET);

    // 切换到下一个位
    current_digit = (current_digit + 1) % 3;
}