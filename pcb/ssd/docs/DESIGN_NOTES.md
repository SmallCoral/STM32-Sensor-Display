# A3 设计依据与待验证项

主控为 STM32C562CET6。A3 原理图包含 68 个器件；PCB 仍是 A2 的 59 个封装，宽压 DCDC、J101 6P 接口与 J102 尚未同步布局布线，不能直接投板。供电与芯片勘误记录在 [C562 迁移核对](C562_MIGRATION.md)，晶振和封装尺寸记录在 [A2 修订](HSE_FOOTPRINT_A2.md)。

## 原始器件文档

项目根目录 `docs/` 中保留以下三份原始器件 PDF；新增主控资料见其中的 `STM32C562CET6_官方资料包_2026-09-16/`：

| 文档 | 本版采用的参数 |
| --- | --- |
| `180长规格书(1).pdf` | NTC 50 kΩ ±1% @25℃，B25/50 =3950 K ±1%，−30～105℃；温度换算优先用厂商 R-T 表 |
| `BTL-004A涡轮霍尔规格书(1).pdf` | 5V；红线电源、黄线脉冲、白线地；F=11Q，660 脉冲/L；高电平 ≥4.7V |
| `20250303远帆5858-1DRWB-10(1).pdf` | 58mm 共阴屏，6 个公共端、10 条段线；双列排距 35.08mm、同列针距 2.54mm |

流量规格书分别出现了 2–12 L/min 和 1–25 L/min 两组量程，本文保留原始数据。有效范围需要向供应商确认并通过实物标定。由 F=11Q 可得 Q=F/11，累计体积 L=N/660，这两个关系仅作初始换算。

## 温度与流量接口

J301：1=3.3V，2=NTC_RAW；外部 NTC 跨接两脚。49.9 kΩ / 0.1% 电阻接信号到地，1 kΩ 与 100 nF 构成 ADC 滤波。公式为 `Rntc = 49.9k × (Vref / Vadc − 1)`；正常使用同一 3.3V 供电和 ADC 参考时为比率测量。开路趋近 0V、短路趋近满量程，固件需判故障，避免直接除以零。

BAV199 用于低漏电钳位：1=A1 接 GND、2=K2 接 3.3V、3=K1/A2 接 ADC。保护范围受串联电阻、异常输入电压和 MCU 注入电流限制，输入端不能承受任意电压。ADC 采用较长采样时间，上电或改变采样配置后等待滤波网络稳定。温度结果还要校正 NTC、参考电阻和安装热耦合带来的误差。

J302：1=红线 5V，2=黄线脉冲，3=白线 GND。10 kΩ 上拉兼容开漏输出，4.7 kΩ / 1 nF 抑制窄毛刺。SN74LVC1G17DCKR 用 3.3V 供电，输入允许 5.5V，输出 3.3V 施密特整形脉冲到 PA1。5V 原始脉冲必须经过该缓冲器后再送入 MCU。

两只连接器暂用 JST XH、2.50mm、立式直插。供应商文档只写了 XHB，无法据此确定塑壳、针序和配套端子，采购前需要核对实物。XH 的 2.50mm 间距不等于 2.54mm。

## 显示驱动

U201 为 Holtek HT16K33，28 引脚 SOP，5V 供电。官方 Rev1.10 第 40 页给出的外形为 1.27mm 脚距、17.70–18.11mm 长、最大 7.62mm 本体宽，对应 KiCad `SOIC-28W_7.5x17.9mm_P1.27mm` 封装。采购时限定为 28 脚版本，兼容型号需另行核对。

PB6/PB7 的 3.3V I²C 经两只 BSS138 转换到 5V，低压侧为 Source、高压侧为 Drain、Gate 接 3.3V。两侧各有 4.7 kΩ 上拉，初次调试用 100 kHz。

| HT16K33 | 驱动脚号 | 串联电阻 | 屏幕段线 | 屏幕脚号 |
| --- | --- | --- | --- | --- |
| ROW0 | 25 | R201 | A | 15 |
| ROW1 | 24 | R202 | B | 14 |
| ROW2 | 23 | R203 | C | 13 |
| ROW3 | 22 | R204 | D | 12 |
| ROW4 | 21 | R205 | E | 11 |
| ROW5 | 20 | R206 | F | 10 |
| ROW6 | 19 | R207 | G | 9 |
| ROW7 | 18 | R208 | H | 8 |
| ROW8 | 17 | R209 | I | 7 |
| ROW9 | 16 | R210 | J | 16 |

COM0～COM5（芯片 2～7 脚）直接接屏幕 COM1～COM6（屏幕 1～6 脚）。COM6/7、ROW10～15 留空。

无地址选择二极管时使用 7 位 I²C 地址 **0x70**。建议初始化：系统振荡器开启 `0x21` → 显示关闭 `0x80` → 清零 16 字节显示 RAM → 低亮度 `0xE0` → 显示开启 / 无闪烁 `0x81`。RAM 每个 COM 占两个字节；第 i 个 COM 的地址为 2i，ROW0～7 对应低字节，ROW8～15 对应高字节。只使用前六个 COM 的低十位，其余保持零。

R201～R210 初值为 330Ω，用来限制首次点亮时的峰值电流。HT16K33 采用扫描驱动，不同颜色 LED 的正向压降也不同，因此扫描平均电流不能直接按直流建议值计算。阻值、PWM 亮度和显示映射都要以样屏测试为准。

## 屏幕封装与布局

自建封装 `SSD:ALS5858_1DRWB_10`：外径 58mm，两排各 8 脚，排距 35.08mm，针距 2.54mm，同排首尾跨度 17.78mm。按供应商正视图，1 脚位于左上，左排 1→8 向下，右排 9→16 向上。资料中没有安装孔尺寸，因此封装只保留引脚和外形轮廓。

A2 的焊盘直径为 1.8mm，钻孔 1.0mm，标称单边焊环 0.40mm。供应商只给了针径公差，没有标称针径，所以孔径仍是暂定值。打样前应测量实物针径，并将 [1:1 对位图](../review/display_footprint_1to1.pdf) 按 100% 比例打印检查；打印时关闭“适合页面”，先测量图中的 20mm 标尺。封装的 58mm 轮廓不是 PCB 板框。

当前 PCB 是 A2 供电方案，尚未同步 A3 的宽压 DCDC 与 J101/J102。J101 已取消 D+/D−，布局时只需保证 VBUS、GND 有足够铜宽，并让 CC1/CC2 的 5.1kΩ 下拉靠近接口。两路 DCDC 的输入电容、自举电容、芯片 GND 和功率电感必须形成最小热回路，SW 铜皮要短小并远离晶振和 NTC_ADC。传感器模拟地与 LED COM 回流不要共用狭窄路径；屏幕下方的器件高度仍需用实物核对。

## USB 电源与器件采购

后级 `VIN_SYS`、F101 与两路 DCDC 仍按 **5～12V** 设计，但 J101 采用的 C668623 产品资料标称 5V/3A，因此 **不得从 J101 输入 12V**。本板不包含 USB PD 协商控制器，连接标准 USB-C 电源时只能按 5V 使用；如要从同一接口输入 12V，必须更换为厂商明确标注额定电压不低于 12V 的连接器。F101 选 Littelfuse `1206L050/24WR` 或等效 1206 自恢复保险丝，保持电流 0.5A、最大工作电压至少 24V。

J101 改用 SHOU HAN `TYPE-C 6P(073)`（LCSC `C668623`）。它是仅供电接口，六个信号焊盘为 A9/B9 VBUS、A12/B12 GND、A5 CC1、B5 CC2；没有 D+/D−，所以原理图已删除 USB 数据网络、D101/USBLC6-2SC6，并将 MCU 的 PA11/PA12 标为空接。封装按供应商数据建立，只有四个外侧镀铜椭圆屏蔽壳固定槽，没有妨碍 VBUS 出线的中间圆形定位孔。程序下载与调试统一使用 J102 / J-Link SWD。

保险丝后的 `VIN_SYS` 同时输入 U101/AP63203 和 U102/AP63205。U101 用 3.9µH 生成独立 +3V3，直接供 MCU、VREF、NTC 分压和 3.3V 逻辑；U102 用 4.7µH 生成 +5V，供 HT16K33 和流量传感器。两路都不级联，因此 5V 显示负载变化不会先经过 MCU 电源。

C101 为 100nF / 25V 高频旁路；C102、C110 是 U101、U102 各自就近的 10µF / 35V 输入电容，输入总标称电容约 20.1µF。C103/C105 与 C107/C108 分别构成两路 2×22µF 输出电容，使用 1206 以保证直流偏压后的有效容量；C106/C109 为 100nF 自举电容。完整样机仍需在 5V 和 12V 输入下测量插入浪涌、输出纹波和负载瞬态。

C401～C403 分别为三个 VDD 的 100nF 去耦，C404 为 4.7µF / 10V 总去耦，C406 为 VREF 的 100nF 旁路。**C405 是 VCAP 专用的 2.2µF 电容**，仅连接 U401-22 与地；额定电压至少 10V，3MHz 时 ESR＜20mΩ，采购时需核对阻抗曲线。上述电容全部为 0603，最终布局必须靠近对应供电脚。

整机输入仍以 0.5A 自恢复保险丝限流，平均和峰值电流留待样机测量。5V 输入时 U102 工作在低压差模式，必须在最大显示亮度和流量传感器负载下确认 +5V 不低于 HT16K33 的 4.5V 下限；首次点屏从低亮度开始。U101 的 +3V3 在后级 5～12V 输入范围内都有足够压差，样机验收应确认 MCU 端为 3.3V±3%、启动单调、满负载纹波建议小于 50mVpp，并检查 J-Link SWD 与 ADC 读数。

SW101、SW102、SW301 改用 Omron `B3U-1000P`（LCSC `C231329`）顶按常开贴片轻触开关；本体约 3.0×2.5mm、高 1.6mm，无定位柱，两端焊盘分别对应开关的 1、2 脚。按帽直径约 1.5mm，裸板适合用指甲按压；如配外壳，应给三个按键设计小顶杆并核对装配行程。J102 改用与 J-Link 插线一致的 JST XH `B4B-XH-A` 立式 4P 接口，标准间距为 2.50mm；线序定义为 1=VREF（板上 +3V3）、2=SWDIO（PA13）、3=SWCLK（PA14）、4=GND。VREF 只用于让 J-Link 检测目标电平，不作为给整板供电的输入；该 4 芯接口不含 NRST，需要连接状态下复位时使用板上 SW102。

## 参考资料与库来源

- [STM32C562xx DS14927 Rev2](reference/STM32C562xx_DS14927_Rev2.pdf)：p37 图5引脚图、p51 表14复用功能、p55 图13供电与 VCAP、LQFP48 封装尺寸；来自用户提供的 ST 官方资料包。
- [AN6274 Rev1](reference/AN6274_STM32C5_Hardware_Rev1.pdf)：§7.4、图10，STM32C5 最小系统及去耦；参考图的 LQFP64 脚号已按 C562 LQFP48 重新映射。
- [AN2606 Rev70](reference/AN2606_Rev70.pdf)：Pattern19、第11章 C551 / C552 / C562 Bootloader，表23 USB 引脚与时钟。使用 PA11 / PA12、内部 HSI÷3 和 CRS；使用启动按键时设置 BOOT_SEL=1。
- [ES0661 Rev1](reference/ES0661_STM32C562_Rev1.pdf)：当前相关的 PC13 / LSE 和 USB 接收限制见迁移说明。
- [STM32C562CET6 LQFP48 引脚表](reference/STM32C562CET6_LQFP48引脚表.csv)：来自用户资料包，并与 DS14927 图5逐项核对；实际外围复用以工程引脚分配为准，例如 PB6 在本版用于 I2C1_SCL。
- [HT16K33 Rev1.10](reference/HT16K33_Rev110.pdf)：Holtek RAM Mapping 16×8 LED Controller Driver，2011-05-16，28-SOP 引脚、RAM 和封装。
- [AP63203 / AP63205](https://www.diodes.com/datasheet/download/AP63200-AP63201-AP63203-AP63205.pdf)：Diodes Incorporated，3.8～32V、固定 3.3V / 5V 同步降压，推荐 3.9µH / 4.7µH、10µF 输入、2×22µF 输出和 100nF 自举电容。
- [Littelfuse 1206L 系列](https://www.littelfuse.com/assetdocs/littelfuse-ptc-1206l-datasheet?assetguid=2b6a1515-d4ee-4c83-8bd4-152b4901b8f5)：F101 的 0.5A 保持电流、24V 最大工作电压和 1206 封装依据。
- [Sunlord SWPA 系列](https://www.sunlordinc.com/uploads/files/20221122/SWPA%20series%20of%20SMD%20Power%20Inductor.pdf)：L101/L102 的电感、电流和官方焊盘尺寸依据。
- [SHOU HAN TYPE-C 6P(073) / C668623](https://jlcpcb.com/partdetail/TYPE-C%206P(073)/C668623)：J101 的 6P 仅供电 Type-C 接口、5V/3A 资料额定与机械焊盘依据；项目封装按供应商数据建立。
- [Omron B3U](https://components.omron.com/us-en/sites/components.omron.com.us/files/datasheet_pdf/A162-E1.pdf)：SW101、SW102、SW301 的 3.0×2.5mm 顶按轻触开关尺寸、额定值和推荐焊盘。
- [SN74LVC1G17](reference/SN74LVC1G17.pdf)：Texas Instruments，`https://www.ti.com/lit/ds/symlink/sn74lvc1g17.pdf`。
- [BAV199 文本摘录](reference/BAV199_reference.txt)：Nexperia，2023-04-01 数据手册；原文 `https://assets.nexperia.com/documents/data-sheet/BAV199.pdf`。
- 标准符号和焊盘来自 KiCad 10.0.6，并复制到项目内的 `libraries/`。标准封装引用 `${KICAD10_3DMODEL_DIR}` 下的 STEP 模型；MCU、晶振和 5858 屏幕使用项目内 STEP。C562、HT16K33 和屏幕符号，以及屏幕封装均为项目自建。库许可见 `https://www.kicad.org/libraries/license/`，文件来源与本地名称映射记录在 `review/design_manifest.json`。
