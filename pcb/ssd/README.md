# STM32 温度 / 流量显示板 A2

主控为 STM32C562CET6：Cortex-M33、最高 144MHz、512KB 闪存、128KB SRAM，LQFP48（7×7mm、0.5mm 脚距）。当前工程包含完整原理图、项目库以及已经布局布线的 PCB。

MCU 焊盘采用 ST 图42的 1.20×0.30mm、0.50mm 脚距；圆屏焊盘直径为 1.8mm，钻孔为 1.0mm。

电阻、电容统一采用 0603 英制 / 1608 公制，原理图注释使用中文。主导航页只表示模块关系，各功能电路放在独立子页中。

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
| [ssd.kicad_pcb](ssd.kicad_pcb) | 59 个封装、板框和当前走线 |
| [封装预览](review/footprints_staged.pdf) | 包含 MCU、去耦电容与新增 24MHz 晶振电路 |
| [BOM.csv](docs/BOM.csv) | 本版器件、采购规格及封装 |
| [设计说明](docs/DESIGN_NOTES.md) | 接口、屏幕映射、机械核对事项 |
| [C562 迁移核对](docs/C562_MIGRATION.md) | 官方资料依据、最小系统清单、勘误说明 |
| [A2 晶振与封装修订](docs/HSE_FOOTPRINT_A2.md) | 晶振选型、ST 专用焊盘及圆屏焊盘修改 |
| [MCU 尺寸图](review/mcu_footprint_dimensioned.pdf) | 48 脚编号、焊盘尺寸及 1:1 对位 |
| [圆屏 1:1 对位图](review/display_footprint_1to1.pdf) | 1.8mm 焊盘 / 1.0mm 钻孔 |

PCB 已有板框、布局和走线，但当前版本尚未重新运行完整 DRC，也没有生成生产文件。现有 59 个封装都已绑定 3D 模型。

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
| NRST 7 | 接 USB 页的 R104、C104、SW102 和 SWD 复位线 |
| PH2-BOOT0 44 | 接 USB 页的 R103 下拉与 SW101 启动按键 |

C405 在 3MHz 时 ESR 必须小于 20mΩ，须检查所购型号的阻抗曲线。每个 VDD 去耦、VCAP 电容和 VREF 旁路必须在最终 PCB 布局中靠近相应引脚；当前暂放位置不是最终电源布局。

板上安装 24MHz HSE 无源晶振，直接连接 PH0 / PH1（5 / 6 脚），供应用程序启用；ROM USB DFU 仍使用内部 HSI / CRS。晶振型号为 ABM8-24.000MHZ-10-D2Y-T，CL=10pF，两只负载电容初值为 12pF / C0G。未用引脚在原理图中标为空接。PA13 / PA14 保留 SWD 功能，但当前 PCB 没有专用调试连接器。

## 外围信号映射

下表按 DS14927 Rev2 图5、引脚表和表14复用功能整理。

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

1. 在 STM32CubeProgrammer 中确认启动相关选项字节。使用启动按键时，将 BOOT_SEL=1，由外部 BOOT0 引脚选择启动模式。若需通过 SWD 修改选项字节，需临时引出调试信号。
2. 按住启动键，按下并释放复位键，再释放启动键。
3. 在 STM32CubeProgrammer 中选择 USB，连接 DFU 设备，下载并校验程序。
4. 释放启动键并复位，从用户闪存启动。

空片进入 ROM 还受 Pattern19 中 BOOT0、BOOT_SEL 和 EMPTY 状态影响，并非所有启动配置都会自动进入 DFU。使用 SWD 时，3.3V 只作为目标板电压参考，避免调试器和板上 LDO 同时向该网络供电。

## 检查状态

- 当前原理图和 PCB 各有 59 个器件；23 个电阻和 19 个电容均为 0603。
- PCB 含 246 个焊盘、305 段走线和 6 个过孔，59 个封装均有 3D 模型绑定。
- [主控引脚核对](review/mcu_pinout_checks.json)记录了 C562 的 48 脚映射和封装焊盘检查。
- `review/` 中的 ERC、网表和 DRC 文件来自较早的 A2 检查点。删除 R401、J102 并重新布局后尚未重新生成，不能作为当前版本的放行依据。
- 屏幕孔径与安装方向、接插件配套、亮度和 USB 涌入电流仍需样机验证。

历史快照包括 [HSE 与封装修订前版本](review/before_hse_footprint_revision.zip) 和 [C562 迁移前版本](review/before_c562_migration.zip)。主控参考资料集中在 [STM32C562CET6 官方资料包](../../docs/STM32C562CET6_官方资料包_2026-09-16/00_README_中文.md)，其中的原始 PDF 未改动。
