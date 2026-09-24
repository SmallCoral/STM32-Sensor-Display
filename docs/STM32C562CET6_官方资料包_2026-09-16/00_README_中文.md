# STM32C562CET6 官方资料包

本目录保存 STM32C562CET6 的 ST 官方文档、LQFP48 引脚表和最小系统器件表，供原理图、PCB 和固件开发查阅。

> 本目录以保存上游资料原貌和校验信息为主。项目实物照片、演示视频及设计说明位于 [项目总览](../../README.md)、[硬件资料说明](../DESIGN_NOTES.md) 和 [媒体索引](../media/README.md)，不计入本资料包的官方文件校验范围。

## 文件索引

| 文件 | 内容 |
| --- | --- |
| `official/01_DS14927_STM32C562xx_Datasheet.pdf` | 芯片特性、引脚、复用功能、电气参数和封装尺寸 |
| `official/02_RM0522_STM32C5_Reference_Manual.pdf` | RCC、GPIO、ADC、定时器、通信接口和系统寄存器 |
| `official/03_PM0264_Cortex-M33_Programming_Manual.pdf` | Cortex-M33 内核、异常、中断、MPU 和调试 |
| `official/04_AN6274_STM32C5_Hardware_Design_Guide.pdf` | 供电、VCAP、时钟、复位、启动和 PCB 布局 |
| `official/05_AN2606_System_Memory_Bootloader.pdf` | ROM Bootloader 支持的接口和启动方式 |
| `official/06_ES0661_STM32C551_552_562_Errata.pdf` | STM32C551/C552/C562 芯片勘误 |
| `official/07_MB2213_NUCLEO-C562RE_Official_Schematic.pdf` | NUCLEO-C562RE 官方原理图 |
| `official/08_NUCLEO-C562RE_Data_Brief.pdf` | NUCLEO-C562RE 开发板简介 |
| `00_STM32C562CET6_LQFP48引脚表.csv` | LQFP48 的 1～48 脚速查表 |
| `00_最小系统BOM.csv` | MCU 最小系统器件清单 |
| `SHA256SUMS.txt` | 文件校验值 |
| `00_官方文件来源.txt` | 官方下载来源 |

## LQFP48 最小系统

| 功能 | 引脚 | 连接 |
| --- | ---: | --- |
| VDD | 24、36、48 | +3.3V，每个电源脚配 100nF 去耦 |
| VSS | 23、35、47 | GND |
| VCAP | 22 | 2.2µF/≥10V 到地，不连接外部负载 |
| VREF+ | 9 | +3.3V，100nF 旁路至 VREF− |
| VREF− | 8 | GND |
| NRST | 7 | RC 复位网络和复位按键 |
| PH2-BOOT0 | 44 | 默认下拉，启动按键拉至 +3.3V |
| SWDIO | 34 / PA13 | J-Link SWDIO |
| SWCLK | 37 / PA14 | J-Link SWCLK |
| HSE | 5/6 / PH0/PH1 | 外部晶振输入和输出 |

MCU 电源范围为 2.7～3.6V。VCAP 是内部内核稳压器输出，外接电容需要靠近引脚，不能作为 3.3V 电源使用。

## 项目引脚分配

| 功能 | MCU 引脚 | 复用 |
| --- | --- | --- |
| NTC 温度采样 | PA0 | ADC1_IN0 |
| 流量脉冲 | PA1 | TIM2_CH2 / EXTI |
| 显示时钟 | PB6 | I2C1_SCL / AF4 |
| 显示数据 | PB7 | I2C1_SDA / AF4 |
| 用户按键 | PC13 | GPIO / EXTI |
| SWD | PA13 / PA14 | SWDIO / SWCLK |
| BOOT0 | PH2 | 系统启动选择 |

本项目的 PA11、PA12 空接，不使用 USB 数据和 USB DFU。Type-C 接口只提供 5V 电源，程序下载和在线调试使用 J-Link SWD。

## 时钟

STM32C562 支持内部 HSI/PSI、4～50MHz HSE 和 32.768kHz LSE。本项目使用 24MHz HSE 晶振，不安装 LSE。HSE 晶体和负载电容靠近 PH0/PH1 放置，周围保持连续地参考。

## 官方链接

- [STM32C562CE 产品页](https://www.st.com/en/microcontrollers-microprocessors/stm32c562ce.html)
- [STM32C5 文档中心](https://www.st.com/en/microcontrollers-microprocessors/stm32c5-series/documentation.html)
- [NUCLEO-C562RE](https://www.st.com/en/evaluation-tools/nucleo-c562re.html)
- [STM32CubeC5](https://www.st.com/en/embedded-software/stm32cubec5.html)
- [STM32CubeProgrammer](https://www.st.com/en/development-tools/stm32cubeprog.html)

目录中的 PDF 保持原始内容，文件名前的编号仅用于排序。

## 在本项目中的落地位置

- MCU 电源、VCAP、复位、BOOT 和 HSE：[`pcb/ssd/mcu_minimum.kicad_sch`](../../pcb/ssd/mcu_minimum.kicad_sch)
- MCU 引脚与最小系统说明：[STM32C562CET6 最小系统](../../pcb/ssd/docs/C562_MIGRATION.md)
- 24MHz HSE 与 LQFP48 封装：[晶振与封装说明](../../pcb/ssd/docs/HSE_FOOTPRINT_A2.md)
- 144MHz 时钟初始化：[`code/Core/Src/system_clock.c`](../../code/Core/Src/system_clock.c)

`SHA256SUMS.txt` 只覆盖资料包中的原始官方文件；编辑或新增项目文档时不要改写其中的校验值。
