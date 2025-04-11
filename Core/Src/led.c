/* LED_Display_Driver.c
 * 针对 LED 数码显示屏的驱动（基于 10×6 矩阵，STM32F103C8T6，PB0～PB15）
 *
 * 假设：共阴极方式（即驱动时，选中的列输出低电平，行侧输出高电平点亮对应 LED）
 * 注意：如果你的数码管为共阳极，则请将以下代码中的电平逻辑反转。
 */

#include "stm32f1xx_hal.h"

/* --- 定义管脚映射 --- */
/* 数码管列：对应 PB0～PB5（图中针脚1～6） */
#define COL0 GPIO_PIN_0   // PB0
#define COL1 GPIO_PIN_1   // PB1
#define COL2 GPIO_PIN_2   // PB2
#define COL3 GPIO_PIN_3   // PB3
#define COL4 GPIO_PIN_4   // PB4
#define COL5 GPIO_PIN_5   // PB5

/* 数码管行：对应 PB6～PB15（图中针脚7～16），分别代表 A～J */
#define ROW0 GPIO_PIN_6   // A
#define ROW1 GPIO_PIN_7   // B
#define ROW2 GPIO_PIN_8   // C
#define ROW3 GPIO_PIN_9   // D
#define ROW4 GPIO_PIN_10  // E
#define ROW5 GPIO_PIN_11  // F
#define ROW6 GPIO_PIN_12  // G
#define ROW7 GPIO_PIN_13  // H
#define ROW8 GPIO_PIN_14  // I
#define ROW9 GPIO_PIN_15  // J

// 用于方便控制的数组定义
GPIO_TypeDef* const COL_PORT = GPIOB;
uint16_t const COL_PINS[6] = {COL0, COL1, COL2, COL3, COL4, COL5};

GPIO_TypeDef* const ROW_PORT = GPIOB;
uint16_t const ROW_PINS[10] = {ROW0, ROW1, ROW2, ROW3, ROW4, ROW5, ROW6, ROW7, ROW8, ROW9};

/* --- 全局变量 --- */
/*
 * display_buffer 用来保存每一列上各行 LED 的状态。
 * 每个元素低 10 位分别对应一列中 A～J 的点亮情况。
 * 例如，如果 display_buffer[2] 的位 0（对应 A）为1，则表示第3列（图中的“3”）上 LED A 点亮。
 */
volatile uint16_t display_buffer[6] = {0, 0, 0, 0, 0, 0};

/* 当前正在扫描的列（0~5） */
volatile uint8_t currentColumn = 0;


/* --- 驱动接口原型 --- */
void LED_Display_Init(void);
void LED_Display_Update(void);     // 刷新函数，需要在定时器中断中周期性调用
void LED_Display_SetBuffer(uint8_t col, uint16_t pattern); // 设置某列的点亮模式
void LED_Display_SetNumber(uint32_t number); // 示例：将数值显示在6位数码管上

/* --- 驱动函数实现 --- */

/* 初始化 GPIO，配置所有连接 LED 矩阵的管脚为推挽输出 */
void LED_Display_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    __HAL_RCC_GPIOB_CLK_ENABLE();

    // 配置列（PB0～PB5）
    GPIO_InitStruct.Pin = COL0 | COL1 | COL2 | COL3 | COL4 | COL5;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    // 配置行（PB6～PB15）
    GPIO_InitStruct.Pin = ROW0 | ROW1 | ROW2 | ROW3 | ROW4 | ROW5 |
                          ROW6 | ROW7 | ROW8 | ROW9;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    // 初始状态：将所有列设为高电平，行设为低电平
    for (int i = 0; i < 6; i++) {
        HAL_GPIO_WritePin(COL_PORT, COL_PINS[i], GPIO_PIN_SET);
    }
    for (int i = 0; i < 10; i++) {
        HAL_GPIO_WritePin(ROW_PORT, ROW_PINS[i], GPIO_PIN_RESET);
    }
}

/*
 * LED_Display_Update()
 *
 * 此函数用于逐列扫描刷新数码管显示，当作定时器中断服务程序中调用（例如1-2毫秒调用一次）。
 *
 * 工作步骤：
 * 1. 将所有列置为非选通（高电平）。
 * 2. 按照当前列的 display_buffer 数据设置各行电平：如果对应位为1，则行输出高电平，否则置低。
 * 3. 将当前列置为选通（输出低电平），使得对应行若为高电平，则该 LED 得到正向驱动而点亮。
 * 4. 切换至下一列，供下一次中断刷新使用。
 */
void LED_Display_Update(void)
{
    /* 1. 将所有列设为非选通（高电平） */
    for (int i = 0; i < 6; i++) {
        HAL_GPIO_WritePin(COL_PORT, COL_PINS[i], GPIO_PIN_SET);
    }

    /* 2. 根据当前列的模式设置行输出（10位数据） */
    for (int i = 0; i < 10; i++) {
        if (display_buffer[currentColumn] & (1 << i)) {
            HAL_GPIO_WritePin(ROW_PORT, ROW_PINS[i], GPIO_PIN_SET);   // 点亮所需行（提供正电平）
        } else {
            HAL_GPIO_WritePin(ROW_PORT, ROW_PINS[i], GPIO_PIN_RESET);   // 不点亮则置低
        }
    }

    /* 3. 选通当前列：共阴极方式，选通列输出低电平 */
    HAL_GPIO_WritePin(COL_PORT, COL_PINS[currentColumn], GPIO_PIN_RESET);

    /* 4. 切换到下一列 (循环扫描: 0->1->2->…->5->0…) */
    currentColumn = (currentColumn + 1) % 6;
}

/*
 * LED_Display_SetBuffer()
 *
 * 设置第 col 列的显示状态，pattern 的低10位分别对应 A～J 行的点亮（1为点亮，0为熄灭）。
 */
void LED_Display_SetBuffer(uint8_t col, uint16_t pattern)
{
    if (col < 6) {
        display_buffer[col] = pattern & 0x03FF;  // 只保留低10位
    }
}

/*
 * 示例：显示一个6位十进制数（例如数码管数字），采用右侧对齐的方式。
 *
 * 这里假设你事先定义了每个数字（0～9）的段码映射表，映射方式为：bit0 ~ bit9 分别代表 A～J 段。
 * 注意：不同的LED显示模块可能接法不同，段码映射需要依据你的实际硬件进行调整。
 */
uint16_t digit_patterns[10] = {
    /* 以下段码仅为示例，按位定义：bit0 = A, bit1 = B, …, bit6 = G, bit7 = H, bit8 = I, bit9 = J
       例如，若数字 0 应点亮 A, B, C, D, E, F (而不点亮 G、H、I、J) ，则对应定义可能为：0b000000111111
       请根据你的数码管实际连接重新定义各数字 */
    0b000000111111, // 0 —— 假设 A, B, C, D, E, F 点亮
    0b000000001100, // 1 —— 假设只点亮 B, C
    0b000001101101, // 2
    0b000001011101, // 3
    0b000001010011, // 4
    0b000001011011, // 5
    0b000001111011, // 6
    0b000000001101, // 7
    0b000001111111, // 8
    0b000001011111  // 9
};

void LED_Display_SetNumber(uint32_t number)
{
    /* 假设需要在6位数码管上显示一个不超过999999的数，低位在右 */
    for (int col = 0; col < 6; col++) {
        uint8_t digit = number % 10;
        LED_Display_SetBuffer(col, digit_patterns[digit]);
        number /= 10;
    }
}

/* --- 示例：定时器中断调用 --- */
/*
 * 例如，如果使用 TIM2 定时器，每当产生更新中断时调用 LED_Display_Update() 刷新显示：
 *
 * void TIM2_IRQHandler(void)
 * {
 *     if (__HAL_TIM_GET_FLAG(&htim2, TIM_FLAG_UPDATE) != RESET)
 *     {
 *         if (__HAL_TIM_GET_IT_SOURCE(&htim2, TIM_IT_UPDATE) != RESET)
 *         {
 *             __HAL_TIM_CLEAR_IT(&htim2, TIM_IT_UPDATE);
 *             LED_Display_Update();
 *         }
 *     }
 * }
 *
 * 定时器参数（周期）需要根据你的刷新需求来调节，一般每个扫描周期 1～2ms 比较合适，由于总共有6列，
 * 整体刷新频率约为 300～600Hz，可确保无闪烁效果。
 */

/* --- 说明 ---
 *
 * 1. 请先确保 HAL 库初始化（包括 HAL_Init()、SystemClock_Config()）以及 GPIO 时钟开启。
 * 2. 根据你的开发环境，定时器中断配置可采用STM32CubeMX自动生成代码后，插入 LED_Display_Update() 调用。
 * 3. 根据实际数码管接法调整数字段码映射（digit_patterns 数组）的定义。
 * 4. 如果你的数码管采用共阳极连接，所有的 HAL_GPIO_WritePin 的逻辑（SET 与 RESET）需要反转：
 *    - 非选通状态时列输出改为低电平
 *    - 选通状态时列输出改为高电平
 *    - 行输出则相应反转
 *
 * 以上代码仅为基础示例，你可以根据自己项目需求扩展更多功能，例如显示字符串、动画效果、亮度调节等。
 */

