<div align="right">

[English](../en/README_SDK_REPLACE.md) | **简体中文**

</div>

# 客户 SDK 申请与配置流程

本文档说明客户如何获取并集成对应型号的 SDK。

---

## 一、申请与替换流程

1. **获取硬件标识码**  
   获取产品的硬件标识码（SN）。

2. **官网申请 SDK**  
   登录 [https://www.daisch.com](https://www.daisch.com) ，提交硬件标识码，选择相应的选项（例如产品型号、SDK 类型）。申请成功后下载 SDK 静态库。

3. **替换库文件**  
   将下载的静态库文件替换到 Demo 工程目录的对应路径下：

   ```text
   lib/<型号>/
   ```

   找到并替换此文件：

   ```text
   DsImuSdk_placeholder.a
   ```

   替换后，**将文件重命名为**：

   ```text
   DsImuSdk_Zephyr_CM33.a
   ```
4. **重新编译工程**  
   进入工程根目录，根据目标 IMU 型号执行对应的 `west build` 命令。

---

## 二、编译命令示例

### 编译 IM3J 版本

```bash
west build -b arduino_uno_q -- -DIMU_TARGET=IM3J
```

### 编译 IM8S 版本

```bash
west build -b arduino_uno_q -- -DIMU_TARGET=IM8S
```

---

## 三、结果验证

编译完成后，将固件烧录至 **Arduino UNO‑Q**，验证SDK初始化成功（`DsImu_Init()`返回`INIT_SUCCESS`）

