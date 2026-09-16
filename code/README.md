# STM32C562 固件占位目录

目标芯片为 **STM32C562CET6**，硬件依据为 [KiCad 工程](../pcb/ssd/README.md)。原 STM32F103 固件已经从当前文件树移除，旧实现仍保存在 Git 历史中。

本目录暂未包含启动代码、HAL、构建系统或应用程序，不能直接编译或烧录。空目录使用 `.gitkeep` 占位。

| 目录 | 后续用途 |
| --- | --- |
| `Core/Inc/` | 芯片初始化与中断等头文件 |
| `Core/Src/` | 芯片初始化、中断与程序入口 |
| `Core/Startup/` | 与 STM32C562 匹配的启动文件 |
| `Drivers/` | 适配 STM32C5 的 CMSIS、HAL / LL 等依赖 |
| `App/` | 温度与流量计算、校准、显示和按键逻辑 |
| `BSP/` | HT16K33、NTC、流量输入等板级接口 |

后续建立固件时，应按 [当前引脚分配](../pcb/ssd/README.md#外围信号映射) 配置 ADC、定时器、I²C、USB 和 SWD。应用使用24MHz HSE时，需显式配置时钟源；出厂ROM的USB DFU使用内部HSI / CRS。LSE目前未安装。

采用新的 STM32C5 工程，不将旧 F1 工程的启动文件、链接脚本或引脚映射当作当前配置。
