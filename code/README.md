# STM32C562 固件

本目录用于 STM32C562CET6 固件。外设连接以 [KiCad 工程说明](../pcb/ssd/README.md) 为准。

## 目录

| 目录 | 内容 |
| --- | --- |
| `Core/Inc/` | 系统初始化、中断和应用头文件 |
| `Core/Src/` | 程序入口、系统初始化和中断处理 |
| `Core/Startup/` | STM32C562 启动文件 |
| `Drivers/` | CMSIS、HAL/LL 和器件驱动 |
| `App/` | 温度换算、流量统计、显示和按键逻辑 |
| `BSP/` | HT16K33、NTC、流量输入等板级接口 |

## 外设配置

| 功能 | 引脚 | 外设 |
| --- | --- | --- |
| NTC 温度采样 | PA0 | ADC1_IN0 |
| 流量脉冲计数 | PA1 | TIM2_CH2 / EXTI |
| 显示 I²C | PB6 / PB7 | I2C1 SCL / SDA |
| 用户按键 | PC13 | GPIO / EXTI |
| J-Link 调试 | PA13 / PA14 | SWDIO / SWCLK |
| 启动按键 | PH2-BOOT0 | BOOT0 |
| 外部晶振 | PH0 / PH1 | 24MHz HSE |

PA11、PA12 在硬件上空接，不配置 USB。板上没有 32.768kHz LSE，时钟树使用内部时钟或 24MHz HSE。

显示驱动 HT16K33 使用 7 位地址 `0x70`。流量传感器标称关系为 `F = 11Q`，其中 F 的单位为 Hz，Q 的单位为 L/min；温度换算使用 NTC 厂商提供的 R-T 表或 B 值公式。
