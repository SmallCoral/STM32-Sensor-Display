# STM32C562 固件占位目录

目标芯片为 STM32C562CET6，接口定义以 [KiCad 工程](../pcb/ssd/README.md) 为准。STM32F103 的旧代码不再放在当前文件树中，可从 Git 历史查阅。

这里目前只有目录结构，没有启动代码、HAL、构建系统或应用程序，因而不能直接编译或烧录。空目录使用 `.gitkeep` 保留。

| 目录 | 后续用途 |
| --- | --- |
| `Core/Inc/` | 芯片初始化与中断等头文件 |
| `Core/Src/` | 芯片初始化、中断与程序入口 |
| `Core/Startup/` | 与 STM32C562 匹配的启动文件 |
| `Drivers/` | 适配 STM32C5 的 CMSIS、HAL / LL 等依赖 |
| `App/` | 温度与流量计算、校准、显示和按键逻辑 |
| `BSP/` | HT16K33、NTC、流量输入等板级接口 |

建立固件工程时，按 [当前引脚分配](../pcb/ssd/README.md#外围信号映射) 配置 ADC、定时器、I²C 和 USB。使用 24 MHz HSE 时需要在时钟树中显式启用；芯片 ROM 内的 USB DFU 使用内部 HSI / CRS。板上没有 LSE。

固件应从 STM32C5 工程开始，旧 F1 工程的启动文件、链接脚本和引脚映射均不适用。
