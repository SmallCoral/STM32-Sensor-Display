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

// 列控制引脚 PB0~PB5（1~6列）
const uint16_t column_pins[6] = {
    GPIO_PIN_0, GPIO_PIN_1, GPIO_PIN_2,
    GPIO_PIN_3, GPIO_PIN_4, GPIO_PIN_5
};

// 行控制引脚 PB6~PB15（I~J = 6~15）
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

// 每个数码位对应的7段LED
typedef struct {
    uint16_t row; // 行引脚
    uint16_t col; // 列引脚
} SegmentPin;

// 百位 - 用列1（PB0）
SegmentPin digit_hundreds[7] = {
    {GPIO_PIN_14, GPIO_PIN_0}, // A1
    {GPIO_PIN_13, GPIO_PIN_0}, // B1
    {GPIO_PIN_12, GPIO_PIN_0}, // C1
    {GPIO_PIN_11, GPIO_PIN_0}, // D1
    {GPIO_PIN_10, GPIO_PIN_0}, // E1
    {GPIO_PIN_9,  GPIO_PIN_0}, // F1
    {GPIO_PIN_8,  GPIO_PIN_0}  // G1
};

// 十位 - 用列2（PB1）
SegmentPin digit_tens[7] = {
    {GPIO_PIN_14, GPIO_PIN_1}, // A2
    {GPIO_PIN_13, GPIO_PIN_1}, // B2
    {GPIO_PIN_12, GPIO_PIN_1}, // C2
    {GPIO_PIN_11, GPIO_PIN_1}, // D2
    {GPIO_PIN_10, GPIO_PIN_1}, // E2
    {GPIO_PIN_9,  GPIO_PIN_1}, // F2
    {GPIO_PIN_8,  GPIO_PIN_1}  // G2
};

// 个位 - 用列3（PB2）
SegmentPin digit_units[7] = {
    {GPIO_PIN_14, GPIO_PIN_2}, // A3
    {GPIO_PIN_13, GPIO_PIN_2}, // B3
    {GPIO_PIN_12, GPIO_PIN_2}, // C3
    {GPIO_PIN_11, GPIO_PIN_2}, // D3
    {GPIO_PIN_10, GPIO_PIN_2}, // E3
    {GPIO_PIN_9,  GPIO_PIN_2}, // F3
    {GPIO_PIN_8,  GPIO_PIN_2}  // G3
};

// A1 ~ I1 对应的行引脚（PB14~PB6）
const uint16_t level_bar_rows[9] = {
    GPIO_PIN_14, // A1
    GPIO_PIN_13, // B1
    GPIO_PIN_12, // C1
    GPIO_PIN_11, // D1
    GPIO_PIN_10, // E1
    GPIO_PIN_9,  // F1
    GPIO_PIN_8,  // G1
    GPIO_PIN_7,  // H1
    GPIO_PIN_6   // I1
};

// 当前显示的数字
static uint16_t current_number = 0;

// 当前柱状图等级（0~9）
static uint8_t level_bar_value = 0;

// 清除所有LED
void ClearAllSegments() {
    for (int i = 0; i < 6; i++) {
        HAL_GPIO_WritePin(GPIOB, column_pins[i], GPIO_PIN_SET);
    }
    for (int i = 0; i < 10; i++) {
        HAL_GPIO_WritePin(GPIOB, row_pins[i], GPIO_PIN_RESET);
    }
}

// 点亮某一段LED
void LightSegment(SegmentPin pin) {
    HAL_GPIO_WritePin(GPIOB, pin.col, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOB, pin.row, GPIO_PIN_SET);
}

// 显示一个数字（0-9）在指定位置的段数组上
void DisplayDigit(uint8_t digit, SegmentPin* segment) {
    if (digit > 9) return;
    uint8_t code = segment_table[digit];
    for (int i = 0; i < 7; i++) {
        if (code & (1 << (6 - i))) {
            LightSegment(segment[i]);
        }
    }
}

// 设置要显示的数字
void DisplayNumber(uint16_t num) {
    if (num > 199) return;
    current_number = num;
}

// 设置柱状图等级（通过温度值计算）
void Set_LevelBar(uint32_t temp) {
    uint8_t level = 0;
    if (temp >= 10) {
        level = (temp - 10) / 10 + 1;
        if (level > 9) level = 9;
    }
    level_bar_value = level;
}

// 动态刷新显示
void RefreshDisplay(void) {
    static uint8_t current_digit = 0;

    ClearAllSegments();

    uint8_t hundreds = current_number / 100;
    uint8_t tens = (current_number % 100) / 10;
    uint8_t units = current_number % 10;

    switch (current_digit) {
        case 0:
            if (current_number >= 100)
                DisplayDigit(hundreds, digit_hundreds);
            break;
        case 1:
            if (current_number >= 10)
                DisplayDigit(tens, digit_tens);
            break;
        case 2:
            DisplayDigit(units, digit_units);
            break;
        case 3:
            // 显示温度等级柱状图
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET); // 激活列1
            for (int i = 1; i < 6; i++) {
                HAL_GPIO_WritePin(GPIOB, column_pins[i], GPIO_PIN_SET);
            }
            for (int i = 0; i < 9; i++) {
                if (i < level_bar_value)
                    HAL_GPIO_WritePin(GPIOB, level_bar_rows[i], GPIO_PIN_SET);
                else
                    HAL_GPIO_WritePin(GPIOB, level_bar_rows[i], GPIO_PIN_RESET);
            }
            break;
    }

    current_digit = (current_digit + 1) % 4; // 四段轮换
}

// 初始化
void LED_Init(void) {
    ClearAllSegments();
    current_number = 0;
    level_bar_value = 0;
}

// 主循环调用的刷新函数
void LED_Process(void) {
    RefreshDisplay();
}

// 暂未使用的接口
void LED_Show(void) {
}
