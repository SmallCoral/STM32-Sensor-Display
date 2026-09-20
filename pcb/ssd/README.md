# STM32 温度与流量显示板

这是 STM32 Sensor Display 的 KiCad 硬件工程。电路由电源、MCU 最小系统、传感器接口和 LED 显示驱动四部分组成。

## 工程文件

| 文件 | 内容 |
| --- | --- |
| [ssd.kicad_pro](ssd.kicad_pro) | KiCad 10 工程入口 |
| [ssd.kicad_sch](ssd.kicad_sch) | 原理图主页面 |
| [mcu_minimum.kicad_sch](mcu_minimum.kicad_sch) | STM32C562 最小系统和 24MHz HSE |
| [usb_power.kicad_sch](usb_power.kicad_sch) | Type-C 电源、双路 DCDC、BOOT、RESET 和 SWD |
| [display.kicad_sch](display.kicad_sch) | HT16K33、I²C 电平转换和圆屏 |
| [sensors.kicad_sch](sensors.kicad_sch) | NTC、霍尔流量传感器和用户按键 |
| [ssd.kicad_pcb](ssd.kicad_pcb) | PCB 布局与布线 |
| [libraries/](libraries/) | 项目符号、封装和 3D 模型 |
| [docs/BOM.csv](docs/BOM.csv) | 器件清单 |

使用 KiCad 10 打开 `ssd.kicad_pro`。工程内自建库名为 `SSD`，3D 模型通过 `${KIPRJMOD}` 相对路径引用。

## 电路结构

```text
5V Type-C
    │
  F101
    │ VIN_SYS
    ├── AP63203 ── +3V3 ── STM32C562 / NTC / 3.3V 逻辑
    └── AP63205 ── +5V  ── HT16K33 / 流量传感器

STM32C562 ── I²C 电平转换 ── HT16K33 ── 58mm LED 圆屏
     ├──── ADC ──── NTC
     ├──── PA1 ──── 流量脉冲缓冲器
     └──── SWD ──── J-Link
```

## 电源接口

J101 为 SHOU HAN TYPE-C 6P(073)，LCSC `C668623`。接口没有 USB 数据焊盘，仅连接 VBUS、GND、CC1 和 CC2。

| 网络 | 内容 |
| --- | --- |
| VBUS | J101 的 A9/B9，5V 输入 |
| CC1/CC2 | 各使用 5.1kΩ 下拉 |
| F101 | 0.5A/24V 自恢复保险丝 |
| VIN_SYS | 保险丝后电源，连接两路 DCDC |
| +3V3 | AP63203WU-7 固定输出 |
| +5V | AP63205WU-7 固定输出 |

C668623 的额定电压为 5V，因此 J101 只用于 5V 供电。AP63203/AP63205 本身支持更高输入电压，但 12V 需要从额定电压合适的独立接口接入。

## MCU 最小系统

| 项目 | 参数 |
| --- | --- |
| MCU | STM32C562CET6 |
| 封装 | LQFP48，7×7mm，0.5mm 脚距 |
| VDD | 3 组 +3V3，每组 100nF 去耦 |
| 总去耦 | 4.7µF/10V |
| VCAP | 2.2µF/10V，低 ESR |
| VREF | +3V3，100nF 旁路至 VREF− |
| HSE | 24MHz ABM8 晶振，6.8pF C0G 负载电容 |
| LSE | 未安装 |

## MCU 信号

| 网络 | MCU 引脚 | 脚号 | 用途 |
| --- | --- | ---: | --- |
| USER_KEY | PC13 | 2 | 用户按键，低电平有效 |
| NTC_ADC | PA0 | 10 | ADC1_IN0 |
| FLOW_PULSE | PA1 | 11 | TIM2_CH2 / EXTI |
| SWDIO | PA13 | 34 | 调试数据 |
| SWCLK | PA14 | 37 | 调试时钟 |
| I2C_SCL | PB6 | 42 | I2C1_SCL / AF4 |
| I2C_SDA | PB7 | 43 | I2C1_SDA / AF4 |
| MCU_BOOT0 | PH2 | 44 | 启动模式选择 |
| HSE | PH0/PH1 | 5/6 | 外部晶振 |

PA11、PA12 空接，不使用 USB 枚举和 DFU。

## SWD 接口

J102 使用 JST XH `B4B-XH-A` 4P 立式连接器，间距 2.50mm。

| 引脚 | 定义 |
| --- | --- |
| 1 | VREF / +3V3 |
| 2 | SWDIO |
| 3 | SWCLK |
| 4 | GND |

VREF 是目标电平参考，不用于给整板供电。复位使用板上的 SW102。

## 传感器接口

J301 连接 50kΩ NTC，1 脚为 +3V3，2 脚为信号。板上使用 49.9kΩ/0.1% 分压电阻、1kΩ 串联电阻、100nF 滤波电容和 BAV199 钳位二极管。

J302 连接 BTL-004A 霍尔流量传感器：1=+5V/红线，2=信号/黄线，3=GND/白线。传感器输出经 RC 滤波和 SN74LVC1G17 施密特缓冲后连接 MCU。

## 显示

U201 使用 HT16K33 驱动 5858-1DRWB-10 圆形 LED 屏。HT16K33 工作在 5V，MCU I²C 工作在 3.3V，两侧通过 BSS138 转换电平。

- I²C 地址：`0x70`
- COM：使用 COM0～COM5
- ROW：使用 ROW0～ROW9
- 段电阻：R201～R210，330Ω
- 显示屏：16 针，2.54mm 针距，两排间距 35.08mm

## PCB 参数

| 项目 | 参数 |
| --- | --- |
| 板层 | 2 层 |
| 板厚 | 1.6mm |
| 最小常用线宽/间距 | 0.20mm / 0.20mm |
| +3V3 | 分支 0.20mm，主干 0.80mm |
| +5V | 0.60mm，局部主干 0.80mm |
| VIN_SYS / SW | 0.80mm |
| 普通过孔 | 0.60mm / 0.30mm 钻孔 |
| 常规阻容 | 0603 |
| DCDC 储能电容 | 1206 |

## 相关文档

- [硬件设计说明](docs/DESIGN_NOTES.md)
- [STM32C562 最小系统](docs/C562_MIGRATION.md)
- [晶振与封装](docs/HSE_FOOTPRINT_A2.md)
- [MCU 封装尺寸图](review/mcu_footprint_dimensioned.pdf)
- [圆屏 1:1 对位图](review/display_footprint_1to1.pdf)
- [STM32C562 官方资料包](../../docs/STM32C562CET6_官方资料包_2026-09-16/00_README_中文.md)
