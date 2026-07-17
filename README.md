# imu_unoq_evk

<div align="center">

[Global Official Site](https://daischsensor.com/) · [中文官网](https://www.daisch.com/WebShop/index.aspx)

</div>

> The IMU-UNO_EVK is an evaluation kit based on the Arduino UNO-Q, designed to help customers quickly evaluate and develop DAISCH's IMU modules. It supports SPI communication, raw data acquisition, full-temperature calibration, attitude estimation, and real-time communication with a host computer. The open-source components include the SPI driver adaptation layer, main program framework, and serial protocol layer, while the core SDK is distributed in closed-source static library form. This kit provides a simple, plug-and-play IMU evaluation environment, reducing the technical barriers for customer selection and accelerating product development progress.

> IMU-UNO_EVK 是基于 Arduino UNO-Q 的评估套件，旨在帮助客户快速评估和开发戴世（DAISCH）的 IMU 模组。它支持 SPI 通信、六轴原始数据采集、全温标定、姿态估计及与上位机实时通信。开源部分包含 SPI 驱动适配层、主程序框架和串口协议层，核心 SDK 以静态库形式闭源分发。本套件提供简单配置即用的 IMU 评估环境，降低客户选型技术门槛，加快产品开发进度。

## 🚀 Prerequisites / 开始前准备

- **Source code of this repository** / **本工程源码**（即当前仓库）
- [**Quick_Start**](./docs/en/Quick_Start.md) / [**快速开始**](./docs/zh-CN/Quick_Start.zh-CN.md)
- **This kit** / **本套件**

## 📖 Documentation / 文档

### English Docs
- [IMU-UNO_EVK Quick Start Guide](./docs/en/Quick_Start.md)
- [IM3J_Configuration_Mapping_Guide](./docs/en/IM3J_Configuration_Mapping_Guide.md)
- [IM8S_Configuration_Mapping_Guide](./docs/en/IM8S_Configuration_Mapping_Guide.md)
- [SDK Replace Guide](./docs/en/README_SDK_REPLACE.md)
- [Data_Passthrough_Configuration](./docs/en/Data_Passthrough_Configuration.md)

### 中文文档
- [IMU-UNO_EVK 快速开始指南](./docs/zh-CN/Quick_Start.zh-CN.md)
- [IM3J 配置映射](./docs/zh-CN/IM3J_Configuration_Mapping_Guide.zh-CN.md)
- [IM8S 配置映射](./docs/zh-CN/IM8S_Configuration_Mapping_Guide.zh-CN.md)
- [SDK 替换指南](./docs/zh-CN/README_SDK_REPLACE.zh-CN.md)
- [数据透传配置](./docs/zh-CN/Data_Passthrough_Configuration.zh-CN.md)

### Project Directory Structure / 工程目录结构

```
.
├── boards/
│   └── arduino_uno_q.overlay       # Zephyr board overlay for Arduino UNO Q
│                                   # 针对 Arduino UNO Q 的 Zephyr 板级覆盖文件
│
├── docs/                           # Documentation / 文档
│   ├── en/                         # English documents / 英文文档
│   ├── images/                     # Image resources / 图片资源
│   └── zh-CN/                      # Chinese documents / 中文文档
│
├── examples/                       # Example source code / 示例源码
│   ├── attitude_output/
│   │   └── attitude_output_example.c    # Attitude (Euler angles) output example
│   │                                    # 姿态（欧拉角）输出示例
│   ├── basic_output/
│   │   └── basic_output_example.c       # Six-axis data output example
│   │                                    # 六轴数据输出示例
│   └── sn_read/
│       └── sn_read_example.c            # Serial number read example
│                                        # 读取序列号示例
│
├── host_tools/
│   └── stm32_to_usb_gadget_bridge.py    # STM32 to USB gadget bridge script
│                                        # STM32 转 USB Gadget
│
├── lib/                            # SDK libraries / SDK 库文件
│   ├── IM3J/                       # Library for IM3J / IM3J
│   │   ├── DsImuSdk.h              # Header file / 头文件
│   │   └── DsImuSdk_placeholder.a  # Placeholder library / 占位库
│   │
│   └── IM8S/                       # Library for IM8S / IM8S
│       ├── DsImuSdk.h
│       └── DsImuSdk_placeholder.a
│
├── src/                            # Source code / 源码
│
├── CMakeLists.txt
├── LICENSE                         # License file / 许可证文件
├── prj.conf                        # Zephyr project Kconfig configuration / Zephyr 项目配置
└── README.md                       # This file / 本文件
```

---

