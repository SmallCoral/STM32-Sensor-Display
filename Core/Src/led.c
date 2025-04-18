#include "led.h"

// 7段数码管段码表：abcdefg（共阴极，1表示点亮）
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

// 列控制引脚 PB0~PB5
const uint16_t column_pins[6] = {
    GPIO_PIN_0, GPIO_PIN_1, GPIO_PIN_2,
    GPIO_PIN_3, GPIO_PIN_4, GPIO_PIN_5
};

// 行控制引脚 PB6~PB15（I~J）
const uint16_t row_pins[10] = {
    GPIO_PIN_6,  // I
    GPIO_PIN_7,  // H
    GPIO_PIN_8,  // G
    GPIO_PIN_9,  // F
    GPIO_PIN_10, // E
    GPIO_PIN_11, // D
    GPIO_PIN_12, // C
    GPIO_PIN_13, // B
    GPIO_PIN_14, // A
    GPIO_PIN_15  // J
};

// 每个数码位对应的7段LED：使用行号和列号定义
typedef struct {
    uint16_t row; // 行引脚
    uint16_t col; // 列引脚
} SegmentPin;

// 7段排列示意（188布局）
SegmentPin digit_left[7] = {
    {GPIO_PIN_14, GPIO_PIN_1}, // A2
    {GPIO_PIN_13, GPIO_PIN_1}, // B2
    {GPIO_PIN_12, GPIO_PIN_1}, // C2
    {GPIO_PIN_11, GPIO_PIN_1}, // D2
    {GPIO_PIN_10, GPIO_PIN_1}, // E2
    {GPIO_PIN_9,  GPIO_PIN_1}, // F2
    {GPIO_PIN_8,  GPIO_PIN_1}  // G2
};

SegmentPin digit_mid[7] = {
    {GPIO_PIN_14, GPIO_PIN_2}, // A3
    {GPIO_PIN_13, GPIO_PIN_2}, // B3
    {GPIO_PIN_12, GPIO_PIN_2}, // C3
    {GPIO_PIN_11, GPIO_PIN_2}, // D3
    {GPIO_PIN_10, GPIO_PIN_2}, // E3
    {GPIO_PIN_9,  GPIO_PIN_2}, // F3
    {GPIO_PIN_8,  GPIO_PIN_2}  // G3
};

SegmentPin digit_right[7] = {
    {GPIO_PIN_14, GPIO_PIN_0}, // A1
    {GPIO_PIN_13, GPIO_PIN_0}, // B1
    {GPIO_PIN_12, GPIO_PIN_0}, // C1
    {GPIO_PIN_11, GPIO_PIN_0}, // D1
    {GPIO_PIN_10, GPIO_PIN_0}, // E1
    {GPIO_PIN_9,  GPIO_PIN_0}, // F1
    {GPIO_PIN_8,  GPIO_PIN_0}  // G1
};

// 清除所有LED
void ClearAllSegments() {
    // 所有列置高
    for (int i = 0; i < 6; i++) {
        HAL_GPIO_WritePin(GPIOB, column_pins[i], GPIO_PIN_SET);
    }

    // 所有行置低
    for (int i = 0; i < 10; i++) {
        HAL_GPIO_WritePin(GPIOB, row_pins[i], GPIO_PIN_RESET);
    }
}

// 点亮某一段LED
void LightSegment(SegmentPin pin) {
    ClearAllSegments(); // 避免重影
    HAL_GPIO_WritePin(GPIOB, pin.col, GPIO_PIN_RESET);  // 列 = 低
    HAL_GPIO_WritePin(GPIOB, pin.row, GPIO_PIN_SET);    // 行 = 高
    HAL_Delay(1); // 稍微延迟点亮
}

// 显示一个数字（0-9）在指定的位置
void DisplayDigit(uint8_t digit, SegmentPin* segment) {
    if (digit > 9) return;
    uint8_t code = segment_table[digit];

    for (int i = 0; i < 7; i++) {
        if (code & (1 << (6 - i))) {
            LightSegment(segment[i]);
        }
    }
}

void ClearDigit(SegmentPin* segment) {
    ClearAllSegments();  // 避免残影
    for (int i = 0; i < 7; i++) {
        HAL_GPIO_WritePin(GPIOB, segment[i].col, GPIO_PIN_RESET);  // 任意拉低
        HAL_GPIO_WritePin(GPIOB, segment[i].row, GPIO_PIN_RESET);  // 全拉低清空
    }
    HAL_Delay(1);
}

// 主函数：显示一个0~199的数字
void DisplayNumber(uint16_t num) {
    if (num > 199) return;

    uint8_t hundreds = num / 100;
    uint8_t tens = (num % 100) / 10;
    uint8_t units = num % 10;

    for (int i = 0; i < 20; i++) {
        if (num >= 100) {
            DisplayDigit(hundreds, digit_right);
        } /*else {
            ClearDigit(digit_right);
        }*/

        if (num >= 10) {
            DisplayDigit(tens, digit_left);
        } /*else {
            ClearDigit(digit_left);
        }*/

        DisplayDigit(units, digit_mid);
    }
}



// 初始化函数（可选）
void LED_Init(void) {
    ClearAllSegments();
}