#include "led.h"
#include "ntc.h"
#include "main.h"
#include "stm32f1xx_it.h"

uint32_t Read_Temperature(void);
uint32_t Read_Flow(void);

uint32_t last_update_time = 0;
uint8_t state = 0;

// 当前LED切换状态：0为D6亮，1为G6亮
static uint8_t led_state = 0;
// 在全局变量区域添加
uint8_t display_celsius = 0; // 默认显示摄氏度
static uint32_t last_key_time = 0;
#define KEY_DEBOUNCE_TIME 200 // 按键消抖时间(ms)

uint32_t ConvertToFahrenheit(uint32_t celsius) {
    return (celsius * 9 / 5) + 32;
}

// 温度变化速率阈值（单位：°C/1000ms，即每1000ms温度升高多少°C）
#define TEMP_RATE_THRESHOLD 2  // 增加温度变化速率阈值，设为0.1°C/秒

static uint32_t last_temperature = 0;  // 上次读取的温度
static uint32_t last_time = 0;         // 上次读取温度的时间


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

// 百位 - 只用 E6 和 F6
SegmentPin digit_hundreds[2] = {
    {GPIO_PIN_10, GPIO_PIN_5}, // E6
    {GPIO_PIN_9,  GPIO_PIN_5}  // F6
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
    {GPIO_PIN_8,  GPIO_PIN_2}, // G3
};

SegmentPin four[7] = {
    {GPIO_PIN_14, GPIO_PIN_3}, // A4
    {GPIO_PIN_13, GPIO_PIN_3}, // B4
    {GPIO_PIN_12, GPIO_PIN_3}, // C4
    {GPIO_PIN_11, GPIO_PIN_3}, // D4
    {GPIO_PIN_10, GPIO_PIN_3}, // E4
    {GPIO_PIN_9,  GPIO_PIN_3}, // F4
    {GPIO_PIN_8,  GPIO_PIN_3}, // G4
};

SegmentPin hot_temp[4] = {
    {GPIO_PIN_7, GPIO_PIN_2}, // H3
    {GPIO_PIN_6, GPIO_PIN_2}, // I3
    {GPIO_PIN_7, GPIO_PIN_3}, // H4
    {GPIO_PIN_6, GPIO_PIN_3}  // I4
};

SegmentPin Conversion[2] = {
    {GPIO_PIN_11, GPIO_PIN_5}, // D6
    {GPIO_PIN_8, GPIO_PIN_5}, // G6
};

SegmentPin up[3] = {
    {GPIO_PIN_14, GPIO_PIN_5}, // A6
    {GPIO_PIN_13, GPIO_PIN_5}, // B6
    {GPIO_PIN_12, GPIO_PIN_5}, // C6
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

void DisplayDigithundreds(uint8_t digit, SegmentPin* segment) {
    if (digit > 9) return;
    for (int i = 0; i < 2; i++) {
        LightSegment(segment[i]);
    }
}

void DisplayHotTemp(SegmentPin* segment) {
    if (level_bar_value > 6) {
        // 点亮 H3, I3, H4, I4 这四个LED
        LightSegment(segment[0]); // H3
        LightSegment(segment[1]); // I3
        LightSegment(segment[2]); // H4
        LightSegment(segment[3]); // I4
    }
}

void DisplayHotLEDsByRate(SegmentPin* segment) {
    LightSegment(segment[0]); // A6
    LightSegment(segment[1]); // B6
    LightSegment(segment[2]); // C6
}

void DisplayConversionC(SegmentPin* segment) {
    LightSegment(segment[1]);
}

void DisplayConversionF(SegmentPin* segment) {
    LightSegment(segment[0]);
}

// 设置要显示的数字
void DisplayNumber(uint16_t num) {
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
            if (current_number >= 100) {
                // 只显示百位的 E6 和 F6
                DisplayDigithundreds(hundreds, digit_hundreds);
            }
            break;
        case 1:
            if (current_number >= 10)
                DisplayDigit(tens, digit_tens);  // 十位显示
            break;
        case 2:
            DisplayDigit(units, digit_units);  // 个位显示
            break;
        case 3:
            // 激活第一列 (GPIO_PIN_0)
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);

            // 逐个点亮前面列控制的 LED
            for (int i = 1; i < 6; i++) {
                HAL_GPIO_WritePin(GPIOB, column_pins[i], GPIO_PIN_SET); // 设置为有效列
            }

            // 控制行（从 A1 到 I1），点亮前 level_bar_value 个 LED
            for (int i = 0; i < 9; i++) {
                if (i < level_bar_value)
                    HAL_GPIO_WritePin(GPIOB, level_bar_rows[i], GPIO_PIN_SET);  // 点亮
                else
                    HAL_GPIO_WritePin(GPIOB, level_bar_rows[i], GPIO_PIN_RESET);  // 关闭
            }
            break;
        case 4:
            // 显示高温状态的 LED
            DisplayHotTemp(hot_temp);
            break;
        case 5:
            if (led_state == 1) {
                // 点亮 D6，熄灭 G6
                DisplayConversionC(Conversion);
            } else {
                // 点亮 G6，熄灭 D6
                DisplayConversionF(Conversion);
            }
            break;
        case 6:
            // 根据温度变化速率判断是否点亮 A6, B6, C6
            if (state == 1) {
                DisplayHotLEDsByRate(up);
            }
            break;
        default:
            break;
    }

    current_digit = (current_digit + 1) % 7;  // 更新显示到下一个数字
}



// 初始化
void LED_Init(void) {
    ClearAllSegments();
    current_number = 0;
    level_bar_value = 0;
}

// 主循环调用的刷新函数
void LED_Process(void) {
    static uint8_t last_state = GPIO_PIN_SET;
    uint8_t current_state = HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13);

    // 检测下降沿（按键按下）
    if (last_state == GPIO_PIN_SET && current_state == GPIO_PIN_RESET) {
        HAL_Delay(KEY_DEBOUNCE_TIME); // 使用宏定义的消抖时间
        if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13) == GPIO_PIN_RESET) {
            display_celsius = !display_celsius; // 切换显示单位
            led_state = !led_state; // 切换LED状态
        }
    }
    last_state = current_state;
    RefreshDisplay();
}


// 暂未使用的接口
void LED_Show(void) {
}

void ssd(void) {
    // 获取传感器温度值（ADC读取）
    uint32_t adc = Read_Temperature();
    uint32_t temp_celsius = NTC_ConvertToCelsius(adc);  // 转换为摄氏度
    uint32_t temp_fahrenheit = ConvertToFahrenheit(temp_celsius);  // 转换为华氏度

    // 获取当前时间
    uint32_t current_time = HAL_GetTick();

    // 计算温度变化速率
    if (current_time - last_time > 1000) { // 每秒更新一次
        float temp_change_rate = (float)(temp_celsius - last_temperature) / (current_time - last_time) * 1000.0f; // °C/秒

        // 如果温度变化速率超过阈值，则调用 DisplayHotLEDsByRate 函数
        if (temp_change_rate > TEMP_RATE_THRESHOLD + 2) {
            state = 0;
        }
        else {
            state = 1;
        }

        // 更新上次温度和时间
        last_temperature = temp_celsius;
        last_time = current_time;
    }

    // 持续刷新显示（轮换）
    LED_Process();

    // 非阻塞更新（每500ms更新一次）
    if (current_time - last_update_time > 500) {
        last_update_time = current_time;

        // 根据显示单位切换
        if (display_celsius) {
            DisplayNumber(temp_celsius);  // 显示摄氏度
        } else {
            DisplayNumber(temp_fahrenheit);  // 显示华氏度
        }

        Set_LevelBar(temp_celsius);  // 设置柱状图
    }
}


