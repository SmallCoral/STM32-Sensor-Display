# STM32 温度 / 流量显示板 · A2

本版主控为 **STM32C562CET6**：Cortex-M33、最高 144MHz、512KB 闪存、128KB SRAM，LQFP48（7×7mm、0.5mm 脚距）。已经完成主控更换、最小系统绘制、封装绑定和 PCB 网络更新。

MCU 焊盘采用 ST 图42的 **1.20×0.30mm**、0.50mm 脚距；圆屏焊盘已减至 **1.8mm**，钻孔仍为1.0mm。

全部电阻、电容采用 **0603 英制 / 1608 公制**，原理图注释为中文。主导航页保留模块关系连线；外围独立分页，页内用实际导线连接。旧 `code/` 未作为本版设计依据。

## 打开与查看

用 KiCad 10 打开 [ssd.kicad_pro](ssd.kicad_pro)。所有使用的符号与封装通过项目内 `SSD` 库绑定，无需另外安装元件库。

| 文件 | 内容 |
| --- | --- |
| [ssd.kicad_sch](ssd.kicad_sch) | 四个功能模块的关系导航 |
| [mcu_minimum.kicad_sch](mcu_minimum.kicad_sch) | U401：STM32C562CET6，完整供电、VCAP、VREF 与接口连接 |
| [usb_power.kicad_sch](usb_power.kicad_sch) | USB-C、电源、ESD、启动 / 复位按键、SWD |
| [display.kicad_sch](display.kicad_sch) | HT16K33、I²C 电平转换、显示屏 |
| [sensors.kicad_sch](sensors.kicad_sch) | 温度、流量、用户按键 |
| [原理图 PDF](review/ssd_schematic.pdf) | 五页完整预览，第五页为新版最小系统 |
| [ssd.kicad_pcb](ssd.kicad_pcb) | 61 个已绑定封装及其网络，按功能暂放 |
| [封装预览](review/footprints_staged.pdf) | 包含 MCU、去耦电容与新增 24MHz 晶振电路 |
| [BOM.csv](docs/BOM.csv) | 本版器件、采购规格及封装 |
| [设计说明](docs/DESIGN_NOTES.md) | 接口、屏幕映射、机械核对事项 |
| [C562 迁移核对](docs/C562_MIGRATION.md) | 官方资料依据、最小系统清单、勘误说明 |
| [A2 晶振与封装修订](docs/HSE_FOOTPRINT_A2.md) | 晶振选型、ST 专用焊盘及圆屏焊盘修改 |
| [MCU 尺寸图](review/mcu_footprint_dimensioned.pdf) | 48 脚编号、焊盘尺寸及 1:1 对位 |
| [圆屏 1:1 对位图](review/display_footprint_1to1.pdf) | 1.8mm 焊盘 / 1.0mm 钻孔 |

PCB 尚未绘制板框或布线，也未生成生产文件。本次修订保留已有 57 个器件的位置和旋转角度；新增晶振、串联电阻和两只负载电容位于右侧暂放区。

## 最小系统已经接好的内容

| 器件 / 引脚 | 连接 |
| --- | --- |
| U401 | `SSD:STM32C562CET6`，封装 `SSD:STM32C562CET6_LQFP48_ST` |
| VDD 24 / 36 / 48 | 全部接 +3V3；C401 / C402 / C403 各 100nF |
| VSS 23 / 35 / 47 | 全部接 GND |
| C404 | 4.7µF / 10V，MCU 总去耦 |
| VCAP 22、C405 | 专用 2.2µF / ≥10V 陶瓷电容到地；不接 3.3V 或外部负载 |
| VREF+ 9、C406 | 接 +3V3，100nF 旁路至 VREF− |
| VREF− 8 | 接 GND |
| NRST 7 | 接 USB 页的 R104、C104、SW102 和 SWD 复位线 |
| PH2-BOOT0 44 | 接 USB 页的 R103 下拉与 SW101 启动按键 |

C405 在 3MHz 时 ESR 必须小于 20mΩ，须检查所购型号的阻抗曲线。每个 VDD 去耦、VCAP 电容和 VREF 旁路必须在最终 PCB 布局中靠近相应引脚；当前暂放位置不是最终电源布局。

本版已安装 24MHz HSE 无源晶振，接 PH0 / PH1（5 / 6 脚），供应用程序启用；ROM USB DFU 仍使用内部 HSI / CRS。晶振为 ABM8-24.000MHZ-10-D2Y-T，CL=10pF；两只负载电容初值12pF / C0G，串联电阻初值0Ω。未用引脚以空接标记注明；固件应将未用 GPIO 配置为确定电平并关闭未用时钟。SWD 保留用于断点调试和首次选项字节配置。

## 外围信号映射

以下映射已按 DS14927 Rev2 图5、引脚表及表14复用功能核对。

| 外围网络 | MCU 引脚 | LQFP48 脚号 | 用途 |
| --- | --- | --- | --- |
| NTC_ADC | PA0 | 10 | ADC1_IN0 |
| FLOW_PULSE | PA1 | 11 | TIM2_CH2 / AF1，或 EXTI |
| USB_DM | PA11 | 32 | USB D− |
| USB_DP | PA12 | 33 | USB D+ |
| I2C_SCL | PB6 | 42 | I2C1_SCL / AF4 |
| I2C_SDA | PB7 | 43 | I2C1_SDA / AF4 |
| USER_KEY | PC13 | 2 | 低电平按下 |
| SWDIO | PA13 | 34 | 调试数据 |
| SWCLK | PA14 | 37 | 调试时钟 |
| MCU_BOOT0 | PH2-BOOT0 | 44 | 启动模式选择 |
| MCU_NRST | NRST | 7 | 复位 |

## USB 供电与烧录

STM32C562 支持出厂 ROM Bootloader 的 USB DFU。AN2606 Rev70 **第11章、表23** 确认 PA11 / PA12 和 HSI÷3 的 48MHz 时钟，DFU 启用 CRS，不要求 HSE 晶振。芯片内置 USB 匹配阻抗及 D+ 上拉，无需另加 22Ω 或 1.5kΩ 电阻。

USB-C 的 CC1、CC2 各有 5.1kΩ 下拉，同一个接口负责 5V 供电与 USB 数据。使用支持数据传输的 USB 线：

1. 首次调试用 SWD 和 STM32CubeProgrammer 检查选项字节；使用启动按键时，将 **BOOT_SEL=1**，由外部 BOOT0 引脚选择启动模式。
2. 按住启动键，按下并释放复位键，再释放启动键。
3. 在 STM32CubeProgrammer 中选择 USB，连接 DFU 设备，下载并校验程序。
4. 释放启动键并复位，从用户闪存启动。

空片进入 ROM 还受 Pattern19 中的 BOOT0、BOOT_SEL 和 EMPTY 状态影响；并非任何启动选项下都会自动进入 DFU。首次调试保留 SWD，不锁定启动选项。SWD 的 3.3V 为目标板电压参考，避免调试器与板上 LDO 同时向该网供电。

## 验证结果与范围

- KiCad 10.0.6 原理图 ERC：**0 错误、0 警告**。
- [网络核对](review/netlist_checks.json)：61 个器件、204 个已连接引脚、54 个独立网络；24 个电阻、19 个电容全部为 0603。
- [PCB 核对](review/pcb_checks.json)：61 个封装、253 个焊盘，焊盘网络、器件参数、封装名称和原理图 UUID 一致，暂放几何不重叠。
- [主控引脚核对](review/mcu_pinout_checks.json)：48 个符号引脚与资料包 CSV 一致，LQFP48 焊盘编号完整，关键电源及接口映射一致。
- [PCB DRC](review/drc_unrouted.json)：原理图一致性问题为 0。未布线、未画板框的提示仍保留；原 USB-C 推荐封装的 4 条孔到铜间距错误仍待结合制板工艺处理，未通过放宽规则隐藏。
- 屏幕孔径与安装方向、接插件配套、实际亮度、USB 涌入电流仍需最终布局和样机验证，详见设计说明。

本次修改前备份为 [before_hse_footprint_revision.zip](review/before_hse_footprint_revision.zip)。C562 迁移前完整工程保存在 [before_c562_migration.zip](review/before_c562_migration.zip)。参考资料优先采用你提供的 [官方资料包](../../docs/STM32C562CET6_官方资料包_2026-09-16/00_README_中文.md)，原始 PDF 未修改。
