# STM32C562 LED 显示演示固件

本目录包含可直接编译的 STM32C562CET6 LL 固件。程序通过 I2C1 驱动 HT16K33，使显示值从 `00` 上升到 `100`、短暂停留后下降到 `00` 并循环。当前代码用于验证时钟、I²C、按键、显示映射和整板点亮效果，尚未读取真实 NTC 与流量传感器。

<p align="center">
  <a href="../docs/media/video/display-demo.mp4">
    <img src="../docs/media/photos/display-lit-dark.jpg" width="430" alt="STM32C562 固件驱动圆形 LED 屏">
  </a>
</p>

<p align="center"><a href="../docs/media/video/display-demo.mp4">播放实机演示视频</a></p>

## 目录

| 目录 | 内容 |
| --- | --- |
| `Core/Inc/` | 系统初始化、中断和应用头文件 |
| `Core/Src/` | 程序入口、系统初始化和中断处理 |
| `Core/Startup/` | STM32C562 官方启动文件 |
| `Drivers/` | ST 官方 CMSIS 和 LL 头文件 |
| `App/` | 数字及进度条往返动画 |
| `BSP/` | I2C1 和 HT16K33 驱动 |
| `Makefile` | GNU Arm Embedded 构建入口 |

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

PA11、PA12 在硬件上空接，不配置 USB。程序使用板载 24MHz HSE 作为 PSI 参考时钟，经 PSIS 输出 144MHz；系统、AHB、APB1、APB2 和 APB3 时钟均为 144MHz。板上不安装 LSE。

## 演示行为

- 上方数字显示演示温度，下方数字显示同一动画值的后两位；这些数值不是传感器实测值。
- 环形 9 段随数值从左向右点亮。
- 数值达到 40℃ 后点亮 `HOT WATER TEMP` 相关段，达到 80℃ 后点亮红色告警区域。
- 按下 `USER` 键可在摄氏和华氏显示之间切换；超过三位显示能力时显示 `HI`。
- I²C 写入失败时，主循环会重新初始化总线和 HT16K33 后继续运行。

## 显示映射

- HT16K33 7 位 I2C 地址：`0x70`，I2C1 内核时钟 144MHz，总线频率 100kHz，`TIMINGR=0x80B25455`。
- 屏幕 A～J 对应 HT16K33 ROW0～ROW9。
- 屏幕 COM1～COM6 对应 HT16K33 COM0～COM5。
- 环形进度条使用 COM1 的 A1～I1，共 9 段。
- 上方两位数字使用 COM2/COM3 的 A～G，下方两位数字使用 COM4/COM5 的 A～G。
- 默认亮度为最高档 16/16；可修改 `BSP/Src/ht16k33.c` 中的 `HT16K33_DEFAULT_BRIGHTNESS`。

主要实现文件：

- [`Core/Src/main.c`](Core/Src/main.c)：初始化、错误重试和按键轮询。
- [`Core/Src/system_clock.c`](Core/Src/system_clock.c)：24MHz HSE 与 144MHz PSIS 时钟配置。
- [`App/Src/display_demo.c`](App/Src/display_demo.c)：段码映射、动画、温标切换和告警逻辑。
- [`BSP/Src/ht16k33.c`](BSP/Src/ht16k33.c)：HT16K33 命令与显示 RAM 更新。
- [`BSP/Src/board_i2c.c`](BSP/Src/board_i2c.c)：PB6/PB7 上的 I2C1 初始化与发送。

## 编译

需要 GNU Arm Embedded 工具链。安装 STM32CubeCLT 或独立的 `arm-none-eabi-gcc` 后，从仓库根目录执行：

```bash
cd code
make -j
```

生成文件位于 `build/`：

- `stm32_sensor_display.elf`
- `stm32_sensor_display.hex`
- `stm32_sensor_display.bin`

程序尚未自动烧录。烧录时请选择器件 `STM32C562CE`、接口 `SWD`，建议初始 SWD 速率为 1MHz。

## 相关资料

- [项目总览](../README.md)
- [硬件工程说明](../pcb/ssd/README.md)
- [显示与传感器电路](../pcb/ssd/docs/DESIGN_NOTES.md)
- [照片与视频索引](../docs/media/README.md)
