# STM32 温度 / 流量显示板 A3 原理图

主控为 STM32C562CET6：Cortex-M33、最高 144MHz、512KB 闪存、128KB SRAM，LQFP48（7×7mm、0.5mm 脚距）。当前工程包含 A3 原理图和项目库；已经布局布线的 PCB 仍是 A2 供电方案，尚未同步新增 DCDC 器件。

MCU 焊盘采用 ST 图42的 1.20×0.30mm、0.50mm 脚距；圆屏焊盘直径为 1.8mm，钻孔为 1.0mm。

常规电阻、电容采用 0603 英制 / 1608 公制，DCDC 的 10µF / 22µF 储能电容采用 1206。原理图注释使用中文；主导航页只表示模块关系，各功能电路放在独立子页中。

## 打开与查看

用 KiCad 10 打开 [ssd.kicad_pro](ssd.kicad_pro)。所有使用的符号与封装通过项目内 `SSD` 库绑定，无需另外安装元件库。

| 文件 | 内容 |
| --- | --- |
| [ssd.kicad_sch](ssd.kicad_sch) | 四个功能模块的关系导航 |
| [mcu_minimum.kicad_sch](mcu_minimum.kicad_sch) | U401：STM32C562CET6，完整供电、VCAP、VREF 与接口连接 |
| [usb_power.kicad_sch](usb_power.kicad_sch) | 6P Type-C 供电、启动 / 复位按键、J-Link SWD |
| [display.kicad_sch](display.kicad_sch) | HT16K33、I²C 电平转换、显示屏 |
| [sensors.kicad_sch](sensors.kicad_sch) | 温度、流量、用户按键 |
| [A2 原理图 PDF](review/ssd_schematic.pdf) | 历史预览，尚未包含 A3 宽压 DCDC |
| [ssd.kicad_pcb](ssd.kicad_pcb) | A2 的 59 个封装、板框和当前走线；尚未同步 A3 电源 |
| [A2 封装预览](review/footprints_staged.pdf) | 历史预览，包含 MCU、去耦电容与 24MHz 晶振电路 |
| [BOM.csv](docs/BOM.csv) | 本版器件、采购规格及封装 |
| [设计说明](docs/DESIGN_NOTES.md) | 接口、屏幕映射、机械核对事项 |
| [C562 迁移核对](docs/C562_MIGRATION.md) | 官方资料依据、最小系统清单、勘误说明 |
| [A2 晶振与封装修订](docs/HSE_FOOTPRINT_A2.md) | 晶振选型、ST 专用焊盘及圆屏焊盘修改 |
| [MCU 尺寸图](review/mcu_footprint_dimensioned.pdf) | 48 脚编号、焊盘尺寸及 1:1 对位 |
| [圆屏 1:1 对位图](review/display_footprint_1to1.pdf) | 1.8mm 焊盘 / 1.0mm 钻孔 |

A2 PCB 已有板框、布局和走线，但不能直接用于 A3 制造。更新电源器件、布局和走线后还需重新运行完整 ERC / DRC；当前没有可放行的生产文件。

## 最小系统

| 器件 / 引脚 | 连接 |
| --- | --- |
| U401 | `SSD:STM32C562CET6`，封装 `SSD:STM32C562CET6_LQFP48_ST` |
| VDD 24 / 36 / 48 | 全部接 +3V3；C401 / C402 / C403 各 100nF |
| VSS 23 / 35 / 47 | 全部接 GND |
| C404 | 4.7µF / 10V，MCU 总去耦 |
| VCAP 22、C405 | 专用 2.2µF / ≥10V 陶瓷电容到地；不接 3.3V 或外部负载 |
| VREF+ 9、C406 | 接 +3V3，100nF 旁路至 VREF− |
| VREF− 8 | 接 GND |
| NRST 7 | 接 USB 页的 R104、C104 和 SW102；J102 不含 NRST |
| PH2-BOOT0 44 | 接 USB 页的 R103 下拉与 SW101 启动按键 |

C405 在 3MHz 时 ESR 必须小于 20mΩ，须检查所购型号的阻抗曲线。每个 VDD 去耦、VCAP 电容和 VREF 旁路必须在最终 PCB 布局中靠近相应引脚；当前暂放位置不是最终电源布局。

板上安装 24MHz HSE 无源晶振，直接连接 PH0 / PH1（5 / 6 脚），供应用程序启用。晶振型号为 ABM8-24.000MHZ-10-D2Y-T，CL=10pF，两只负载电容初值为 12pF / C0G。未用引脚在原理图中标为空接；PA11 / PA12 未连接 USB 数据。PA13 / PA14 通过 J102 引出为 SWDIO / SWCLK。

## 外围信号映射

下表按 DS14927 Rev2 图5、引脚表和表14复用功能整理。

| 外围网络 | MCU 引脚 | LQFP48 脚号 | 用途 |
| --- | --- | --- | --- |
| NTC_ADC | PA0 | 10 | ADC1_IN0 |
| FLOW_PULSE | PA1 | 11 | TIM2_CH2 / AF1，或 EXTI |
| 未使用 | PA11 | 32 | 空接 |
| 未使用 | PA12 | 33 | 空接 |
| I2C_SCL | PB6 | 42 | I2C1_SCL / AF4 |
| I2C_SDA | PB7 | 43 | I2C1_SDA / AF4 |
| USER_KEY | PC13 | 2 | 低电平按下 |
| SWDIO | PA13 | 34 | 调试数据 |
| SWCLK | PA14 | 37 | 调试时钟 |
| MCU_BOOT0 | PH2-BOOT0 | 44 | 启动模式选择 |
| MCU_NRST | NRST | 7 | 复位 |

## Type-C 供电与 J-Link 烧录

J101 使用 SHOU HAN `TYPE-C 6P(073)`（LCSC `C668623`），只连接 VBUS、GND、CC1、CC2；CC1、CC2 各有 5.1kΩ 下拉。接口没有 D+/D−，USBLC6-2SC6 数据 ESD 器件也已删除，因此不能进行 USB 枚举或 DFU。

C668623 的产品资料标称 5V/3A，J101 只能作为 5V 输入。后级 `VIN_SYS`、F101 和两路 DCDC 仍按 5～12V 设计；若要输入 12V，必须另选额定电压不低于 12V 的连接器，不能直接给当前 J101 加 12V。

下载与在线调试使用 J102 和 J-Link。J102 为 JST XH 4P，线序为 1=VREF/+3V3、2=SWDIO、3=SWCLK、4=GND。VREF 仅用于检测目标电平，不能给整板供电；接口不含 NRST，需要复位时使用板上 SW102。

## 检查状态

- A3 原理图和 BOM 包含 68 个器件；3.3V / 5V 分别由 AP63203 / AP63205 从 5～12V 输入独立降压。
- A2 PCB 仍含 59 个封装、246 个焊盘、305 段走线和 6 个过孔，59 个封装均有 3D 模型绑定；A3 电源尚未同步。
- [主控引脚核对](review/mcu_pinout_checks.json)记录了 C562 的 48 脚映射和封装焊盘检查。
- `review/` 中的 PDF、ERC、网表和 DRC 文件来自较早的 A2 检查点，不能作为 A3 的放行依据。
- 更新 PCB 后需在合规输入连接器接入 5V / 12V 和最大负载下确认 MCU 端 3.3V 为 3.3V±3%，启动单调，纹波建议小于 50mVpp；当前 C668623 的 J101 只允许 5V。
- 屏幕孔径与安装方向、接插件配套、亮度和 Type-C 5V 插入浪涌仍需样机验证。

历史快照包括 [HSE 与封装修订前版本](review/before_hse_footprint_revision.zip) 和 [C562 迁移前版本](review/before_c562_migration.zip)。主控参考资料集中在 [STM32C562CET6 官方资料包](../../docs/STM32C562CET6_官方资料包_2026-09-16/00_README_中文.md)，其中的原始 PDF 未改动。
