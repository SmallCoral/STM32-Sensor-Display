# 硬件设计说明

本项目是一块 STM32C562CET6 温度与流量显示板。MCU 采集 NTC 温度和霍尔流量脉冲，通过 HT16K33 驱动 58mm 圆形 LED 屏。

<p align="center">
  <img src="../../../docs/media/photos/assembled-board-powered.jpg" width="620" alt="安装圆屏并点亮的 STM32 Sensor Display 样板">
</p>

实物已经验证供电、MCU、时钟、SWD、I²C、按键与显示链路。当前仓库固件运行的是显示测试动画，照片中的数值不代表真实温度或流量。

## 系统组成

| 功能 | 器件 |
| --- | --- |
| 主控 | STM32C562CET6，LQFP48 |
| 3.3V 电源 | AP63203WU-7 |
| 5V 电源 | AP63205WU-7 |
| 温度输入 | 50kΩ NTC、BAV199 |
| 流量输入 | BTL-004A、SN74LVC1G17DCKR |
| 显示驱动 | HT16K33，28-SOP |
| I²C 电平转换 | BSS138 ×2 |
| 显示屏 | 5858-1DRWB-10 |
| 外部时钟 | ABM8 24MHz 晶振 |
| 调试 | J-Link SWD |

## 电源

J101 使用 SHOU HAN `TYPE-C 6P(073)`，LCSC 编号 `C668623`。接口只保留 VBUS、GND、CC1 和 CC2，CC1/CC2 各接 5.1kΩ 下拉电阻。

J101 的额定电压为 5V，只能作为 5V 电源入口。F101 为 0.5A/24V 自恢复保险丝，保险丝后的网络为 `VIN_SYS`。

`VIN_SYS` 同时连接两路同步降压：

| 电源轨 | 芯片 | 电感 | 输入电容 | 输出电容 | 用途 |
| --- | --- | --- | --- | --- | --- |
| +3V3 | AP63203WU-7 | 3.9µH | 10µF/35V | 2×22µF/10V | MCU、VREF、NTC 和 3.3V 逻辑 |
| +5V | AP63205WU-7 | 4.7µH | 10µF/35V | 2×22µF/10V | HT16K33 和流量传感器 |

两路 DCDC 的自举电容均为 100nF。AP63205 在 5V 输入时工作于低压差模式，输出电压会随输入和负载产生少量压降。

## MCU 最小系统

U401 使用项目封装 `SSD:STM32C562CET6_LQFP48_ST`。封装本体为 7×7mm，0.5mm 脚距，焊盘尺寸按 ST DS14927 推荐图制作。

| 网络 | 连接 |
| --- | --- |
| VDD 24/36/48 | +3V3，每组配 100nF 去耦 |
| VSS 23/35/47 | GND |
| VCAP 22 | C405 2.2µF/10V 到地 |
| VREF+ 9 | +3V3，C406 100nF 到 VREF− |
| VREF− 8 | GND |
| NRST 7 | 10kΩ 上拉、100nF 到地、RESET 按键 |
| PH2-BOOT0 44 | 10kΩ 下拉、BOOT 按键拉至 +3V3 |

C401～C403 为 VDD 去耦，C404 为 4.7µF 总去耦，C405 为 VCAP 专用电容，C406 为参考电压旁路。

Y401 使用 Abracon `ABM8-24.000MHZ-10-D2Y-T`，连接 PH0/PH1。C407、C408 为 6.8pF C0G 负载电容，晶体外壳焊盘接地。板上不安装 LSE 晶振。

## 温度输入

J301 为两针 JST XH 接口：

| 引脚 | 定义 |
| --- | --- |
| 1 | +3V3 |
| 2 | NTC 信号 |

外部 50kΩ NTC 与 R301 49.9kΩ/0.1% 构成分压。R302 1kΩ 和 C301 100nF 组成 ADC 输入滤波，BAV199 将 ADC 节点钳位在电源轨附近，信号连接到 PA0/ADC1_IN0。

NTC 阻值计算：

```text
Rntc = 49.9k × (Vref / Vadc - 1)
```

温度换算可使用传感器规格书中的 R-T 表，也可使用 B25/50=3950K 的 B 值公式。

## 流量输入

J302 为三针 JST XH 接口：

| 引脚 | 线色 | 定义 |
| --- | --- | --- |
| 1 | 红 | +5V |
| 2 | 黄 | 脉冲输出 |
| 3 | 白 | GND |

传感器信号由 R303 10kΩ 上拉至 +5V，经 R304 4.7kΩ 和 C302 1nF 滤波后进入 SN74LVC1G17。缓冲器使用 3.3V 供电，输出 `FLOW_PULSE` 连接 PA1。

BTL-004A 的标称关系为：

```text
F = 11Q
Q = F / 11
```

F 的单位为 Hz，Q 的单位为 L/min；累计流量按 660 个脉冲/L 换算。

## 显示驱动

U201 为 HT16K33，使用 +5V 供电。MCU 侧 I²C 为 3.3V，经 Q201/Q202 两只 BSS138 转换到 5V；高、低压侧均使用 4.7kΩ 上拉。

| HT16K33 | 串联电阻 | 屏幕段线 | 屏幕脚号 |
| --- | --- | --- | --- |
| ROW0 | R201 | A | 15 |
| ROW1 | R202 | B | 14 |
| ROW2 | R203 | C | 13 |
| ROW3 | R204 | D | 12 |
| ROW4 | R205 | E | 11 |
| ROW5 | R206 | F | 10 |
| ROW6 | R207 | G | 9 |
| ROW7 | R208 | H | 8 |
| ROW8 | R209 | I | 7 |
| ROW9 | R210 | J | 16 |

COM0～COM5 对应屏幕 COM1～COM6。R201～R210 均为 330Ω。HT16K33 使用 7 位地址 `0x70`。

## 按键与调试

SW101、SW102、SW301 使用 Omron `B3U-1000P` 小型贴片按键，分别用于 BOOT、RESET 和用户输入。

J102 使用 JST XH `B4B-XH-A` 4P 立式接口，间距 2.50mm：

| 引脚 | 定义 |
| --- | --- |
| 1 | VREF / +3V3 |
| 2 | SWDIO / PA13 |
| 3 | SWCLK / PA14 |
| 4 | GND |

VREF 只用于调试器检测目标电平。J102 不含 NRST，复位由板上 SW102 完成。

## PCB 与封装

- 常规阻容：0603 英制 / 1608 公制。
- DCDC 储能电容：1206 英制 / 3216 公制。
- 信号线：0.20mm。
- +3V3 分支：0.20mm，电源主干使用 0.80mm。
- +5V：0.60mm，局部主干使用 0.80mm。
- VIN_SYS 与两路 SW：0.80mm。
- 普通过孔：0.60mm 外径、0.30mm 钻孔。

显示屏封装 `SSD:ALS5858_1DRWB_10` 外径 58mm，两排各 8 脚，排距 35.08mm，同排针距 2.54mm。焊盘直径 1.8mm，钻孔 1.0mm，1 脚使用方形焊盘标识。

## 实物检查入口

| 检查对象 | 资料 |
| --- | --- |
| 元件位置与焊接外观 | [未安装圆屏的 PCB 正面](../../../docs/media/photos/assembled-board-front.jpg) |
| 圆屏安装与整板点亮 | [成品板点亮照片](../../../docs/media/photos/assembled-board-powered.jpg) |
| 发光区域、颜色和可读性 | [暗环境显示照片](../../../docs/media/photos/display-lit-dark.jpg) |
| 数字、图标、环形段与按键 | [动态演示视频](../../../docs/media/video/display-demo.mp4) |
| 元件定位与 BOM 核对 | [交互式 BOM](../bom/ibom.html) |

媒体用于记录样板状态和辅助装配检查，不作为尺寸测量依据；封装尺寸仍以规格书、KiCad 封装和 1:1 对位图为准。

## 参考资料

- [项目照片与视频](../../../docs/media/README.md)
- [5858-1DRWB-10 外形与矩阵预览](../../../docs/media/reference/display-spec-mechanics-and-matrix.png)
- [5858-1DRWB-10 光电参数预览](../../../docs/media/reference/display-spec-optical-electrical.png)
- [STM32C562xx DS14927](reference/STM32C562xx_DS14927_Rev2.pdf)
- [STM32C5 硬件设计指南 AN6274](reference/AN6274_STM32C5_Hardware_Rev1.pdf)
- [STM32 系统 Bootloader AN2606](reference/AN2606_Rev70.pdf)
- [STM32C562 勘误 ES0661](reference/ES0661_STM32C562_Rev1.pdf)
- [HT16K33 数据手册](reference/HT16K33_Rev110.pdf)
- [AP63203/AP63205 数据手册](https://www.diodes.com/datasheet/download/AP63200-AP63201-AP63203-AP63205.pdf)
- [C668623 连接器资料](https://jlcpcb.com/partdetail/TYPE-C%206P(073)/C668623)
