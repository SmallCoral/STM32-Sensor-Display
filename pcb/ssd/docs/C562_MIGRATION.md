# STM32C562CET6 最小系统

本项目使用 STM32C562CET6，LQFP48 封装。器件基于 Arm Cortex-M33，最高主频 144MHz，片上 512KB Flash 和 128KB SRAM。

<p align="center">
  <img src="../review/mcu_footprint_3d.png" width="560" alt="STM32C562CET6 LQFP48 封装 3D 预览">
</p>

## 参考资料

| 文档 | 用途 |
| --- | --- |
| [DS14927](reference/STM32C562xx_DS14927_Rev2.pdf) | 引脚、复用功能、电气参数和 LQFP48 封装 |
| [AN6274](reference/AN6274_STM32C5_Hardware_Rev1.pdf) | 电源、去耦、时钟、复位和 PCB 布局 |
| [AN2606](reference/AN2606_Rev70.pdf) | ROM Bootloader 和启动方式 |
| [ES0661](reference/ES0661_STM32C562_Rev1.pdf) | STM32C562 芯片勘误 |
| [LQFP48 引脚表](reference/STM32C562CET6_LQFP48引脚表.csv) | 1～48 脚速查 |

## 电源连接

| 引脚 | 网络 | 外围器件 |
| ---: | --- | --- |
| 24、36、48 | +3V3 | 每个 VDD 配 100nF 去耦 |
| 23、35、47 | GND | VSS |
| 22 | VCAP | 2.2µF/≥10V 低 ESR 电容到地 |
| 9 | VREF+ | +3V3，100nF 旁路 |
| 8 | VREF− | GND |

C401～C403 为三个 VDD 的 100nF 去耦，C404 为 4.7µF 总去耦，C405 为 VCAP 电容，C406 为 VREF 旁路。VCAP 是内部内核稳压器输出，不能连接 3.3V 或外部负载。

## 时钟

Y401 为 Abracon `ABM8-24.000MHZ-10-D2Y-T` 无源晶振，连接 PH0-OSC_IN 和 PH1-OSC_OUT。

| 器件 | 参数 |
| --- | --- |
| Y401 | 24MHz，CL=10pF，基频 |
| C407/C408 | 6.8pF，C0G，0603 |
| Y401-2/4 | GND |

板上不安装 32.768kHz LSE。PC14、PC15 保持空接，PC13 用作用户按键。

## 启动与复位

NRST 通过 10kΩ 上拉至 +3V3，并使用 100nF 电容到地。SW102 按下时将 NRST 拉低。

PH2-BOOT0 通过 10kΩ 下拉至 GND。SW101 按下时将 BOOT0 拉至 +3V3，用于选择系统启动模式。

## 调试接口

PA13 和 PA14 保留 SWD 功能，通过 J102 引出：

| J102 | MCU | 信号 |
| ---: | --- | --- |
| 1 | +3V3 | VREF |
| 2 | PA13 | SWDIO |
| 3 | PA14 | SWCLK |
| 4 | GND | GND |

## 外设分配

| 功能 | MCU 引脚 | 复用 |
| --- | --- | --- |
| NTC ADC | PA0 | ADC1_IN0 |
| 流量脉冲 | PA1 | TIM2_CH2 / AF1，或 EXTI |
| 用户按键 | PC13 | GPIO / EXTI |
| 显示 I²C | PB6/PB7 | I2C1 SCL/SDA，AF4 |
| SWD | PA13/PA14 | SWDIO/SWCLK |

PA11、PA12 空接。Type-C 接口不包含 D+、D−，固件不启用 USB 数据功能。

## 芯片勘误说明

- PC13 的跳变可能影响 LSE。本项目不安装 LSE，因此 PC13 用作按键输入。
- LSE 低驱动模式受勘误限制；本项目不使用 LSE。
- USB 接收缓冲区相关限制只适用于 USB 数据功能，本项目 PA11/PA12 空接。
- PB6 在本项目中分配给 I2C1_SCL，不使用 FDCAN。

## 实物与固件对应

- 实物板 MCU 与周边布局见[焊接完成的 PCB 照片](../../../docs/media/photos/assembled-board-front.jpg)。
- 24MHz HSE 到 144MHz PSIS 的初始化见 [`code/Core/Src/system_clock.c`](../../../code/Core/Src/system_clock.c)。
- PB6/PB7 的 I2C1 初始化见 [`code/BSP/Src/board_i2c.c`](../../../code/BSP/Src/board_i2c.c)。
- 显示与按键实机运行效果见[演示视频](../../../docs/media/video/display-demo.mp4)。

当前样板已验证 HSE、SWD、I²C、按键和显示链路；ADC 温度采样与流量计数尚未在演示固件中启用。
