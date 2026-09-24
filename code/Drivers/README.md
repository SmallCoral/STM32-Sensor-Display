# ST 官方驱动来源

本目录保存固件构建所需的 CMSIS Core、STM32C5 器件头文件和 STM32C5 LL 驱动。文件取自 STMicroelectronics 官方 STM32CubeC5 上游仓库，导入日期为 2026-09-24。

| 上游组件 | 版本或提交 |
| --- | --- |
| `STM32CubeC5` | `cadccf77d4e5033a5d823bbc4ac1503466e4563a` |
| `stm32c5xx-drivers` | 2.1.0，`a6288ffbf8824171323b8a7ac2214b70de54b3d8` |
| `stm32c5xx-dfp` | DFP 2.1.0，`a5f65bc64535cfa723e9d25f58d7ce23d0937aed` |
| `cmsis-core` | `2327f7224ff212b2436e5a4cadda3288143fd041` |

各组件的原始许可证文件保存在对应目录中，导入代码时请一并保留。项目自己的启动文件、链接脚本和应用代码位于 `Core/`、仓库根下的链接脚本及其他固件目录，构建方式见 [固件说明](../README.md)。

上游入口：

- [STM32CubeC5](https://github.com/STMicroelectronics/STM32CubeC5)
- [STM32C5 Drivers](https://github.com/STMicroelectronics/stm32c5xx-drivers)
- [STM32C5 Device Family Pack](https://github.com/STMicroelectronics/stm32c5xx-dfp)
- [ST CMSIS Core 镜像](https://github.com/STMicroelectronics/cmsis-core)
