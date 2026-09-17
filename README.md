# STM32 Sensor Display

这是一个基于 STM32C562CET6 的温度与流量显示板，外接 NTC 温度探头、霍尔流量传感器和 58 mm 圆形 LED 屏。USB-C 同时提供 5 V 电源和 USB DFU 下载通道。

仓库中的硬件版本为 A2。原理图、项目库和 PCB 已更新到当前器件方案，PCB 中已有板框、布局和走线。现阶段仍需重新运行 ERC / DRC，并通过样板确认电气和机械尺寸。`code/` 只保留固件目录结构；旧 STM32F103 程序可从 Git 历史查阅。

## 硬件方案

| 模块 | 当前选型 / 设计 |
| --- | --- |
| 主控 U401 | STM32C562CET6，Cortex-M33，最高144MHz，512KB Flash、128KB SRAM |
| 主控封装 | LQFP48，7×7mm本体、0.5mm脚距，四边各12脚；焊盘按 ST DS14927 图42制作 |
| 应用时钟 | 24MHz HSE 无源晶振；未配置 LSE 晶振；应用固件需自行启用相应时钟源 |
| 供电 | USB-C 5V输入，AP2112K-3.3产生3.3V；USB接口带ESD保护与自恢复保险丝 |
| 温度输入 | 50kΩ NTC，B25/50=3950K；分压、滤波后接 ADC |
| 流量输入 | BTL-004A 霍尔传感器，5V供电；经滤波与3.3V施密特缓冲后接 MCU |
| 显示屏 | 5858-1DRWB-10，58mm共阴圆屏，6个公共端、10条段线 |
| 显示驱动 U201 | Holtek HT16K33，28脚SOP、两侧各14脚；I²C经BSS138进行3.3V/5V电平转换 |
| 下载与调试 | USB DFU、启动/复位按键；PA13 / PA14 保留 SWD 信号，板上暂未放专用 SWD 接口 |
| 电阻与电容 | 全部采用 **0603英制 / 1608公制** |

HSE 为应用程序提供外部时钟参考；ROM USB DFU 仍按官方 Bootloader 设计使用内部 HSI 配合 CRS。无需为 USB DFU 额外安装32.768kHz晶振。

## 文件树

仓库分为 `code/`、`docs/` 和 `pcb/`：

```text
STM32-Sensor-Display/
├── README.md
├── code/                         # STM32C562 固件占位目录
│   ├── README.md
│   ├── Core/
│   │   ├── Inc/
│   │   ├── Src/
│   │   └── Startup/
│   ├── Drivers/
│   ├── App/
│   └── BSP/
├── docs/                         # 传感器、屏幕原始资料及 MCU 官方资料包
│   ├── BOM.csv
│   ├── DESIGN_NOTES.md
│   ├── reference/
│   └── STM32C562CET6_官方资料包_2026-09-16/
└── pcb/
    └── ssd/
        ├── ssd.kicad_pro         # 从这里打开 KiCad 工程
        ├── ssd.kicad_sch         # 主导航页
        ├── mcu_minimum.kicad_sch # MCU 最小系统与24MHz晶振
        ├── usb_power.kicad_sch   # USB、电源、启动、复位、SWD
        ├── display.kicad_sch     # 显示驱动与屏幕
        ├── sensors.kicad_sch     # 温度、流量、用户按键
        ├── ssd.kicad_pcb
        ├── libraries/           # 项目内符号、封装与部分STEP模型
        ├── docs/                # 硬件说明、BOM、引用资料
        ├── review/              # PDF 预览、阶段性检查报告和历史快照
        └── tools/               # 网表与PCB核对脚本
```

空目录用 `.gitkeep` 保留。编辑器缓存、KiCad 锁文件和个人界面状态不提交。

## 打开工程

1. 克隆仓库：

   ```bash
   git clone https://github.com/SmallCoral/STM32-Sensor-Display.git
   cd STM32-Sensor-Display
   ```

2. 使用 **KiCad 10** 打开 [pcb/ssd/ssd.kicad_pro](pcb/ssd/ssd.kicad_pro)。
3. 打开原理图，在主导航页双击模块进入对应子页。
4. 打开 PCB 可查看当前的 59 个封装、板框和走线。

符号和封装通过工程内 `SSD` 库引用；项目自建的 3D 模型使用 `${KIPRJMOD}` 相对路径。克隆后应保留 `pcb/ssd/` 内部目录结构。查看硬件工程不需要 STM32 开发环境。

## 文档与预览

| 文件 | 内容 |
| --- | --- |
| [硬件工程说明](pcb/ssd/README.md) | 供电、最小系统、信号分配及USB下载说明 |
| [完整原理图 PDF](pcb/ssd/review/ssd_schematic.pdf) | 五页原理图，第五页为MCU最小系统 |
| [PCB 封装暂放预览](pcb/ssd/review/footprints_staged.pdf) | 当前器件封装集合 |
| [BOM](pcb/ssd/docs/BOM.csv) | 器件规格、封装和采购核对事项；下单前应与当前原理图复核 |
| [设计依据与待验证项](pcb/ssd/docs/DESIGN_NOTES.md) | 传感器接口、显示映射、机械尺寸和样机验证 |
| [C562 迁移说明](pcb/ssd/docs/C562_MIGRATION.md) | 官方资料依据、供电与相关勘误 |
| [A2 晶振与封装修订](pcb/ssd/docs/HSE_FOOTPRINT_A2.md) | HSE选型、MCU专用焊盘及圆屏焊盘调整 |
| [MCU 封装尺寸与脚号图](pcb/ssd/review/mcu_footprint_dimensioned.pdf) | 48脚编号、焊盘尺寸及1:1对位 |
| [圆屏1:1对位图](pcb/ssd/review/display_footprint_1to1.pdf) | 1.8mm焊盘、1.0mm钻孔；按100%打印核对 |
| [STM32C562 官方资料包](docs/STM32C562CET6_官方资料包_2026-09-16/00_README_中文.md) | 数据手册、参考手册、硬件指南、Bootloader说明与勘误 |

`docs/BOM.csv` 和 `docs/DESIGN_NOTES.md` 为根资料目录中的同步副本；与原理图配套的完整硬件文档位于 `pcb/ssd/docs/`。

## 固件接口规划

| 功能 | MCU引脚 | LQFP48脚号 |
| --- | --- | --- |
| NTC温度 ADC | PA0 | 10 |
| 流量脉冲 | PA1 | 11 |
| USB D− / D+ | PA11 / PA12 | 32 / 33 |
| I²C SCL / SDA | PB6 / PB7 | 42 / 43 |
| 用户按键 | PC13 | 2 |
| SWDIO / SWCLK | PA13 / PA14 | 34 / 37 |
| BOOT0 / NRST | PH2-BOOT0 / NRST | 44 / 7 |
| HSE输入 / 输出 | PH0 / PH1 | 5 / 6 |

固件尚未实现，没有可直接编译的 CubeMX、CMake 或 IDE 工程。后续应使用适配 STM32C5 的设备支持包，按当前原理图重新建立工程，详见 [code/README.md](code/README.md)。

## 当前状态

A2 阶段曾保存过 ERC、网表和封装检查报告。此后原理图删去了 R401 和 J102，PCB 也已重新布局，因此 `review/` 中的旧报告只能作为过程记录。

- 当前原理图包含 59 个器件，其中 23 个电阻、19 个电容，阻容封装均为 0603。
- 当前 PCB 包含 59 个封装、246 个焊盘、305 段走线和 6 个过孔。
- 59 个 PCB 封装都已绑定 3D 模型；DS201 使用项目内 STEP 模型。
- MCU 的 48 个引脚名称、编号和封装焊盘已按资料核对。

检查报告位于 [pcb/ssd/review/](pcb/ssd/review/)。原理图改动后，可在 `pcb/ssd/` 目录重新导出网表并检查：

```bash
kicad-cli sch export netlist --format kicadxml -o review/ssd.net.xml ssd.kicad_sch
kicad-cli sch erc --format json -o review/erc.json ssd.kicad_sch
python3 tools/check_netlist.py
```

`tools/check_pcb.py` 需要 KiCad 的 `pcbnew` Python 模块。提交制造文件前，应重新生成网表并运行 ERC、DRC 和项目检查脚本。

## 尚未完成

- 当前版本的整板 ERC / DRC、丝印和制造文件检查；USB-C 封装的孔到铜间距还需按制板能力确认。
- 显示屏针径、连接器配套和机械空间的实物核对。
- 晶振起振、负载电容、USB涌入电流、显示电流及传感器测量的样机验证。
- STM32C562固件、USB应用枚举、传感器校准与显示逻辑。

仓库目前不提供可直接投产的制造文件或经过样机验证的固件。厂商资料、KiCad 库和 STEP 模型沿用各自的版权及许可，来源记录在硬件设计说明中。
