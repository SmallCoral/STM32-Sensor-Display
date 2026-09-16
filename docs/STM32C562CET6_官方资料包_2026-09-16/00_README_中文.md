# STM32C562CET6 官方资料包

整理日期：2026-09-16

本资料包对应 **STM32C562CET6**：STM32C5 系列、Cortex-M33、144 MHz、512 KB Flash、128 KB SRAM、LQFP48、温度等级 6（环境温度 -40 至 +85 °C，结温最高 +105 °C）。

## 建议阅读顺序

1. `official/01_DS14927_STM32C562xx_Datasheet.pdf`
   - 选型、LQFP48 引脚、电气参数、封装尺寸和复用功能。
   - 最常用：Figure 5（LQFP48 引脚图）、Table 7（引脚定义）、USB 电气参数、LQFP48 封装尺寸。
2. `official/04_AN6274_STM32C5_Hardware_Design_Guide.pdf`
   - 画板前必看：供电、VCAP、时钟、BOOT、复位、SWD 与参考设计。
   - Figure 10 是 ST 官方 STM32C562 最小系统参考，原图使用 C562RE/LQFP64；可按数据手册引脚映射到 C562CE/LQFP48。
3. `official/07_MB2213_NUCLEO-C562RE_Official_Schematic.pdf`
   - ST 官方 NUCLEO-C562RE 完整原理图，包含 MCU 供电、USB-C、SWD/ST-LINK、FDCAN 等实用电路。
   - 该板为 C562RE/LQFP64，不能照抄引脚号；电路拓扑可参考。
4. `official/05_AN2606_System_Memory_Bootloader.pdf`
   - 查 USB DFU、USART、SPI、FDCAN ROM Bootloader。
   - C551/C552/C562 对应第 11 章；USB DFU 使用 PA11/PA12。
5. `official/06_ES0661_STM32C551_552_562_Errata.pdf`
   - 芯片已知限制与规避办法。定版 PCB、冻结固件前必须核对。
6. `official/02_RM0522_STM32C5_Reference_Manual.pdf`
   - 寄存器、Flash、RCC、GPIO、ADC、FDCAN、USB 等外设的完整说明。
7. `official/03_PM0264_Cortex-M33_Programming_Manual.pdf`
   - Cortex-M33 内核、异常、中断、MPU、指令与调试相关内容。

`00_STM32C562CET6_LQFP48引脚表.csv` 是按 DS14927 Figure 5 整理的 1～48 脚速查表；`00_最小系统BOM.csv` 是首版最小系统板的建议清单。

## 最小系统关键连接

| 功能 | STM32C562CET6 引脚 | 推荐连接 |
|---|---:|---|
| VDD | 24、36、48 | 全部接 3.3 V；每脚就近 100 nF |
| VSS | 23、35、47 | 全部接地 |
| VCAP | 22 | 2.2 µF 到地，紧贴芯片；不得接 3.3 V、不得给外部电路供电 |
| VREF+ | 9 | 接 3.3 V，旁路 100 nF；模拟精度要求高时按数据手册优化 |
| VREF- | 8 | 接地 |
| NRST | 7 | 100 nF 到地，复位按键到地；内部已有上拉 |
| BOOT0 | 44 / PH2-BOOT0 | 10 kΩ 下拉，按键或跳帽拉到 3.3 V |
| SWDIO | 34 / PA13 | 接 SWD 调试器 SWDIO |
| SWCLK | 37 / PA14 | 接 SWD 调试器 SWCLK |
| USB D- | 32 / PA11 | 直接接 USB D-，靠近接口放 USB ESD 器件 |
| USB D+ | 33 / PA12 | 直接接 USB D+，靠近接口放 USB ESD 器件 |
| FDCAN RX | 41 / PB5 | 可接 3.3 V FDCAN 收发器 RXD |
| FDCAN TX | 42 / PB6 | 可接 3.3 V FDCAN 收发器 TXD |

MCU 附近除 3 个 100 nF 外，再放至少 **4.7 µF** 的总去耦。所有 VDD/VSS 均必须连接。电源范围为 **2.7～3.6 V**，USB VBUS 的 5 V 不得直接接 MCU VDD，必须经过 3.3 V 稳压器。

DS14927 对 VCAP 电容还给出了额外约束：2.2 µF、在 3 MHz 时 ESR < 20 mΩ、额定电压至少 10 V。实际选型时应核对电容在 3.3 V 直流偏压下的有效容量。

## USB-C 与 USB DFU

- PA11 = USB_DM，PA12 = USB_DP。
- USB-C 作为设备端（UFP）时，CC1、CC2 各接一个 5.1 kΩ 到地。
- C562 内置 USB D+/D- 匹配阻抗和 D+ 上拉，通常不需要外置 22 Ω 串联电阻，也不要再加 1.5 kΩ D+ 上拉。
- 芯片 ROM Bootloader 支持 USB DFU。AN2606 第 11 章说明：USB FS 强制为 Device 模式，DFU 使用 HSI/3 得到 48 MHz，并启用 CRS，因此仅做 DFU 不强制要求外部 HSE。
- C5 的启动由 BOOT_SEL、BOOT0 选项位/引脚、BOOTADD 和 EMPTY 状态共同决定。首次空白芯片可因 EMPTY 进入 Bootloader；若希望以后稳定实现“按住 BOOT + 复位进入 DFU”，应在 STM32CubeProgrammer 中正确配置相关 Option Bytes。
- USB DFU 只能烧录/擦除，不能替代 SWD 的断点、单步和寄存器调试，因此仍建议保留 3V3、GND、SWDIO、SWCLK、NRST 五个调试信号。

## 首版 PCB 注意事项

- USB D+/D- 等长、并行、少过孔，远离开关电源与高 dv/dt 节点；ESD 器件紧靠接口。
- VCAP 2.2 µF、每个 VDD 的 100 nF 都应紧靠对应引脚，回地路径要短。
- 建议保留 HSE 与 LSE 焊盘但默认 DNP；USB DFU 本身不依赖外部晶振。
- FDCAN 仍需外部收发器，MCU 的 TX/RX 不能直接连接 CANH/CANL。120 Ω 终端只在总线端点启用。
- LQFP48 只有 38 个可用 I/O，USB、SWD、FDCAN 和晶振会占用复用资源；画板前先在 STM32CubeMX2 做一次完整 Pinout 冲突检查。
- 生产前重新下载最新版 DS14927、RM0522、AN2606、AN6274 和 ES0661，尤其要复核勘误表。

## 官方链接

- 产品页：https://www.st.com/en/microcontrollers-microprocessors/stm32c562ce.html
- C5 文档中心：https://www.st.com/en/microcontrollers-microprocessors/stm32c5-series/documentation.html
- NUCLEO-C562RE：https://www.st.com/en/evaluation-tools/nucleo-c562re.html
- STM32CubeC5：https://www.st.com/en/embedded-software/stm32cubec5.html
- STM32CubeProgrammer：https://www.st.com/en/development-tools/stm32cubeprog.html

官方 PDF 保持原文件内容，文件名前的编号仅用于排序。
