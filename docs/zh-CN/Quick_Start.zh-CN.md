<div align="right">

[English](../en/Quick_Start.md) | **简体中文**

</div>

---

# 🚀 IMU-UNO_EVK 快速开始指南

本指南介绍如何基于 **IMU-UNO_EVK** ，通过六个步骤完成从硬件安装到IMU数据输出的流程。

**步骤概览：**
1. [搭建开发环境](#section1)
2. [硬件连接](#section2)
3. [SDK 替换](#section3)
4. [项目构建](#section4)
5. [固件烧录](#section5)
6. [数据透传配置](#section6)
7. [上位机查看数据](#section7)

**IMU-UNO_EVK 系统框图:**

<img src="../images/IMU-UNO_EVK.png" alt="IMU-UNO_EVK 系统框图" width="60%">

---

## 📌 一、开发环境<a name="section1"></a>

本项目开发环境如下：

* **操作系统**：Ubuntu 22.04
* **Zephyr 版本**：v4.3.0
* **Zephyr SDK**：v1.0.0

### 🔧 环境搭建

请参考 Zephyr 官方文档完成环境配置：

* 👉 Linux 依赖安装
  [https://docs.zephyrproject.org/latest/develop/getting_started/installation_linux.html#installation-linux](https://docs.zephyrproject.org/latest/develop/getting_started/installation_linux.html#installation-linux)

* 👉 Zephyr 环境初始化
  [https://docs.zephyrproject.org/latest/develop/getting_started/index.html](https://docs.zephyrproject.org/latest/develop/getting_started/index.html)

---

## 🔌 二、硬件连接<a name="section2"></a>

### 📍 1. 硬件展示

<img src="../images/Accessories.jpg" alt="Accessories" width="60%">

---

### 📍 2. 硬件安装
本套件采用堆叠安装，安装方式如图所示：

<img src="../images/Installation Instruction.png" alt="Installation Instruction" width="60%">

---

## 📦 三、SDK 替换<a name="section3"></a>

请参考：

[README_SDK_REPLACE.zh-CN.md](README_SDK_REPLACE.zh-CN.md)


---

## ⚙️ 四、项目构建<a name="section4"></a>

### 🧩 条件编译

进入项目根目录。

根据 IMU 型号选择编译目标：

#### ✔ 编译 IM3J

```bash
west build -b arduino_uno_q -- -DIMU_TARGET=IM3J
```
>在获取正式的 SDK 静态库之前，可使用随附的“空壳库”（仅包含头文件和空函数定义）验证编译流程与环境配置，命令如下：west build -b arduino_uno_q -- -DIMU_TARGET=IM3J_STUB

#### ✔ 编译 IM8S

```bash
west build -b arduino_uno_q -- -DIMU_TARGET=IM8S
```
>在获取正式的 SDK 静态库之前，可使用随附的“空壳库”（仅包含头文件和空函数定义）验证编译流程与环境配置，命令如下：west build -b arduino_uno_q -- -DIMU_TARGET=IM8S_STUB

---

### 📁 编译输出

编译完成后生成：

```bash
build/zephyr/zephyr.bin
```

---

## 🔥 五、固件烧录<a name="section5"></a>

### 1️⃣ 传输固件
使用含有Type-C接口的数据线连接PC，如图：

<img src="../images/UNO_Q To PC.jpg" alt="UNO_Q To PC" width="60%">

通过ADB将 `zephyr.bin` 传输到UNO_Q：

```bash
adb push "/path/to/zephyr.bin" /path/to/device/
```

---

### 2️⃣ 进入 OpenOCD 目录

```bash
cd /opt/openocd
```

---

### 3️⃣ 烧录命令

```bash
sudo /bin/openocd \
  -f /openocd_gpiod.cfg \
  -c "init; reset halt; flash write_image erase /path/to/zephyr.bin 0x08000000; reset; shutdown"
```

---

## 🔧 六、数据透传配置<a name="section6"></a>

详见 [数据透传配置](../zh-CN/Data_Passthrough_Configuration.zh-CN.md)

---

## ✅ 七、上位机查看数据<a name="section7"></a>

**打开上位机并连接套件：**

* Tpye-C 输出 IMU 数据
* 可获取 **姿态角（Roll / Pitch / Yaw）、六轴加角速度** 等数据

<img src="../images/DS_RVision.png" alt="DS_RVision" width="60%">


**IMU参数配置：**

可在右上角的“设备信息配置”中进行参数设置。

<img src="../images/DS_RVision_2.png" alt="DS_RVision_2" width="60%">


具体的配置项说明请参考以下文档：
- [IM3J_Configuration_Mapping_Guide.zh-CN.md](IM3J_Configuration_Mapping_Guide.zh-CN.md)
- [IM8S_Configuration_Mapping_Guide.zh-CN.md](IM8S_Configuration_Mapping_Guide.zh-CN.md)

<img src="../images/DS_RVision_3.png" alt="DS_RVision_3" width="60%">

---

