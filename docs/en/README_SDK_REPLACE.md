<div align="right">

**English** | [简体中文](../zh-CN/README_SDK_REPLACE.zh-CN.md)

</div>

# Customer SDK Application and Configuration Process
This document describes how customers can obtain and integrate the SDK for the corresponding product model.

## 1. Application and Replacement Process
1. **Obtain Hardware Identifier**
   Obtain the product's hardware identifier code (SN).

2. **Apply for SDK on Official Website**
   Log in to [https://www.daisch.com](https://www.daisch.com) , submit the hardware identifier code, and select the appropriate options (e.g., product model, SDK type). After successful application, download the SDK static library.

3. **Replace Library File**
   Replace the downloaded static library file in the corresponding directory of the Demo project:
   ```text
   lib/<model>/
   ```
   Locate and replace this file:
   ```text
   DsImuSdk_placeholder.a
   ```
   After replacement, **rename the file to**:
   ```text
   DsImuSdk_Zephyr_CM33.a
   ```

4. **Rebuild Project**
   Navigate to the project root directory and execute the corresponding `west build` command based on the target IMU model.

## 2. Compilation Command Examples
### Compile for IM3J version
```bash
west build -b arduino_uno_q -- -DIMU_TARGET=IM3J
```

### Compile for IM8S version
```bash
west build -b arduino_uno_q -- -DIMU_TARGET=IM8S
```

## 3. Result Verification
After compilation is complete, flash the firmware onto the **Arduino UNO‑Q** board. Verify that the SDK initialization is successful (i.e., `DsImu_Init()` returns `INIT_SUCCESS`).