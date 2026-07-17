<div align="right">

[English](../en/IM8S_Configuration_Mapping_Guide.md) | **简体中文**

</div>

# IM8S 配置映射说明

> 宏定义前标注（*）的项为默认项

### 陀螺仪量程
| 宏定义 | 量程 | DS_RVision 下拉框数值 |
| :--- | :--- | :--- |
| IM8S_FS_GY_125_DPS | ±125 dps | 0x00 |
| (*)IM8S_FS_GY_250_DPS | ±250 dps | 0x01 |
| IM8S_FS_GY_500_DPS | ±500 dps | 0x02 |
| IM8S_FS_GY_1000_DPS | ±1000 dps | 0x03 |
| IM8S_FS_GY_2000_DPS | ±2000 dps | 0x04 |

### 陀螺仪 LPF
| 宏定义 | 截止频率 | DS_RVision 下拉框数值 |
| :--- | :--- | :--- |
| IM8S_GY_LPF_274HZ | 274 Hz | 0x00 |
| IM8S_GY_LPF_212HZ | 212 Hz | 0x01 |
| IM8S_GY_LPF_150HZ | 150 Hz | 0x02 |
| IM8S_GY_LPF_390HZ | 390 Hz | 0x03 |
| IM8S_GY_LPF_99HZ | 99 Hz | 0x04 |
| (*)IM8S_GY_LPF_50HZ | 50 Hz | 0x05 |
| IM8S_GY_LPF_25HZ | 25 Hz | 0x06 |
| IM8S_GY_LPF_12HZ | 12 Hz | 0x07 |
| IM8S_GY_LPF_DISABLE | 禁用 | 0x08 |

### 加速度计量程
| 宏定义 | 量程 | DS_RVision 下拉框数值 |
| :--- | :--- | :--- |
| IM8S_FS_XL_2_G | ±2 g | 0x00 |
| IM8S_FS_XL_4_G | ±4 g | 0x01 |
| (*)IM8S_FS_XL_8_G | ±8 g | 0x02 |
| IM8S_FS_XL_16_G | ±16 g | 0x03 |

### 加速度计 LPF
| 宏定义 | 截止频率 | DS_RVision 下拉框数值 |
| :--- | :--- | :--- |
| IM8S_XL_LPF_417HZ | 417 Hz | 0x00 |
| IM8S_XL_LPF_167HZ | 167 Hz | 0x01 |
| IM8S_XL_LPF_83HZ | 83 Hz | 0x02 |
| IM8S_XL_LPF_37HZ | 37 Hz | 0x03 |
| (*)IM8S_XL_LPF_16HZ | 16 Hz | 0x04 |
| IM8S_XL_LPF_8HZ | 8 Hz | 0x05 |
| IM8S_XL_LPF_4HZ | 4 Hz | 0x06 |
| IM8S_XL_LPF_2HZ | 2 Hz | 0x07 |
| IM8S_XL_LPF_DISABLE | 禁用 | 0x08 |

### 加速度计 ODR 配置
| 宏定义 | ODR | DS_RVision 下拉框数值 |
| :--- | :--- | :--- |
| (*)BYPASS | Bypass | 0x08 |