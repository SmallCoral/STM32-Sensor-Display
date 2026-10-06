# STM32C562 温度与流量显示固件

本目录为 STM32C562CET6 的产品应用固件，通过 PA0 读取 50kΩ NTC，通过 PA1 的上升沿中断采集 BTL-004A 流量脉冲，再通过 I2C1 驱动 HT16K33 圆屏。显示值来自传感器，已移除数值往返演示动画。

## 产品行为

- 上方显示实测温度，默认 ℃，短按 USER 切换 ℃/℉；上电默认恢复 ℃。
- 下方显示瞬时流量，单位 L/min，四舍五入为整数。内部保留 mL/min 分辨率。
- WATER FLOW 与 L/min 标签常亮；收到流量脉冲时点亮工作图标，连续 2 秒无脉冲后流量及工作图标归零。
- 环形 9 段按 0～100℃ 点亮，与温标切换无关。
- 温度达到 40℃ 点亮热水提示，降至 38℃ 以下熄灭；达到 80℃ 点亮红色告警，降至 78℃ 以下熄灭。
- 启动采样尚未稳定显示 `--`；NTC 开路、短路或 ADC 故障显示 `Er` 并点亮告警。连续 3 次采样确认状态，重新接好后自动恢复。
- 探头超出 −30～105℃ 的规格书阻温表范围显示 `Lo`/`HI`。上方可显示 −9～199 的整数，超出屏幕数字能力也显示 `Lo`/`HI`，不会截取高位后显示错误数值。
- 显示通信失败后每 500ms 重试，并发送 I²C 总线恢复时钟。传感器采集在显示失败期间继续工作；ADC 初始化失败每秒重试。
- 独立看门狗约 8 秒超时，主循环运行时持续喂狗；调试暂停时冻结看门狗。

累计流量以 660 个脉冲/L 换算，在 RAM 中统计，可从调试变量读取；断电后清零，不在当前屏幕轮播，也不写入 Flash。

## 采样与硬件

| 功能 | 引脚 / 外设 | 配置 |
| --- | --- | --- |
| 温度 | PA0 / ADC1_IN0 | 12 位、9MHz ADC 时钟、289 周期采样，16 次平均，每 50ms 更新 |
| 流量 | PA1 / EXTI1 | 上升沿计数；TIM2 1MHz 时间戳，最近 8 个周期平均 |
| 显示 | PB6/PB7 / I2C1 | 100kHz，HT16K33 地址 0x70，每 200ms 刷新 |
| 按键 | PC13 | 10ms 轮询、约 30～40ms 消抖 |
| 调试 | PA13/PA14 | SWD，J-Link 初始速率 1MHz |
| 时钟 | PH0/PH1 | 24MHz HSE，PSIS 144MHz；各总线 144MHz |

NTC 和 ADC 参考共用 +3V3，换算采用 `Rntc = 49900 × (4095 / ADC - 1)`。正常 ADC 样本先做 1/8 IIR 滤波，再由 `docs/180长规格书(1).pdf` 的 **Rnor** 列按 −30～105℃、1℃ 间隔线性插值求温度。无效原始值在滤波前检测，避免滤波掩盖断线或短路。

流量采用厂商 `F=11Q`，Q 单位 L/min。小于 1.5ms 的重复上升沿作为干扰丢弃，允许正常 1～25L/min 范围。没有独立的传感器在线信号，因此静止水流和未接流量传感器均显示 0。

| 传感器接口 | 引脚定义 |
| --- | --- |
| J301，两针 NTC | 1=+3V3，2=信号；两根黑线无极性 |
| J302，三针流量 | 1=红线/+5V，2=黄线/信号，3=白线/GND |

以元件面朝向观察者、圆屏在上、USB-C 在右为基准：左侧上方 J302 的线色从上到下为白、黄、红，最下方方形焊盘为 1 脚；左侧下方 J301 为 NTC。

## 目录

- `Core/`：启动、时钟、1ms 调度和主循环。
- `BSP/`：ADC、流量中断、TIM2、看门狗、按键及 I²C/HT16K33。
- `App/Src/sensor_model.c`：阻温表、滤波、故障确认、温标及流量换算。
- `App/Src/product_display.c`：屏幕段码、真实读数和告警显示。
- `Core/Inc/product_diagnostics.h`：J-Link 可读的运行诊断结构。
- `tests/test_product.c`：可在电脑运行的计算与显示测试。
- `tools/`：Windows 构建、测试和烧录入口。
- `docs/VALIDATION.md`：本次验证记录及待完成的实物验证。

## 构建和测试

使用 Arm GNU Toolchain 14.3.Rel1 和 GNU Make：

```sh
cd code
make -j4
make test
```

`make test` 使用本机 C 编译器 `cc`，可用 `HOST_CC=gcc` 覆盖。它覆盖厂商阻温表基准点、整个 ADC 有效区间的温度单调性、断线/短路与恢复、告警滞回、流量/累计量/32 位脉冲计数溢出，以及正常值、负数、故障和溢出的屏幕段码。

本机已安装的工具位于 `%LOCALAPPDATA%\Programs\STM32SensorTools`，Windows PowerShell 可直接运行：

```powershell
cd code
.\tools\build.ps1
.\tools\test.ps1
.\tools\flash.ps1
```

构建脚本可传 `-ToolRoot` 指定工具目录，测试脚本可传 `-Compiler` 指定 TCC、GCC 或 Clang。烧录脚本可传 `-JLinkPath` 和 `-Serial`，默认采用官方 J-Link 安装目录、STM32C562CE、SWD、1MHz。J-Link 对下载内容执行 Flash 校验，失败时脚本报错。

编译生成 `build/stm32_sensor_display.elf/.hex/.bin`，产物不提交 Git。GitHub Actions 对 `code/` 的变更执行本机测试和 ARM 编译，并保存固件构建产物。

## 调试

`g_product_diagnostics` 提供运行毫秒数、原始 ADC、温度状态与 0.1℃ 温度、mL/min 流量、接受/拒绝脉冲、累计毫升数高低字、ADC/显示错误数、显示刷新数和温标。按 ELF 符号定位地址；不要固定使用旧版本的 RAM 地址。

NTC 状态枚举见 `App/Inc/sensor_model.h`：0 正常、1 启动、2 开路、3 短路、4 过冷、5 过热、6 ADC 故障。当前 J-Link 进行运行状态内存快照时需先暂停再恢复 MCU；固件在空闲时使用 WFI。

## 资料

- [实机验证记录](docs/VALIDATION.md)
- [硬件设计](../pcb/ssd/docs/DESIGN_NOTES.md)
- [官方驱动来源](Drivers/README.md)
- [SEGGER 官方下载](https://www.segger.com/downloads/jlink/)
- [Arm GNU 工具链](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads)

仓库原有照片和视频记录早期显示演示，不能作为本固件的真实传感器测量录像。
