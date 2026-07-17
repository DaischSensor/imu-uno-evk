<div align="right">

[English](../en/IM3J_Configuration_Mapping_Guide.md) | **简体中文**

</div>

# IM3J 配置映射说明

> 宏定义前标注（*）的项为默认项

### 陀螺仪量程

| 宏定义 | 量程  | DS_RVision 下拉框数值  |
| --- | --- | --- |
| `IM3J_FS_GY_125_DPS` | ±125 dps | 0x00 |
| `IM3J_FS_GY_250_DPS` | ±250 dps | 0x01 |
| `IM3J_FS_GY_500_DPS` | ±500 dps | 0x02 |
| `(*)IM3J_FS_GY_1000_DPS` | ±1000 dps | 0x03 |
| `IM3J_FS_GY_2000_DPS` | ±2000 dps | 0x04 |

### 陀螺仪 LPF / ODR

| 宏定义 | 配置  | DS_RVision 下拉框数值  |
| --- | --- | --- |
| `IM3J_GY_ODR_2000_LPF_532_HZ` | 2000 Hz / 532 Hz | 0x00 |
| `IM3J_GY_ODR_2000_LPF_230_HZ` | 2000 Hz / 230 Hz | 0x01 |
| `IM3J_GY_ODR_1000_LPF_116_HZ` | 1000 Hz / 116 Hz | 0x02 |
| `IM3J_GY_ODR_400_LPF_47_HZ` | 400 Hz / 47 Hz | 0x03 |
| `(*)IM3J_GY_ODR_200_LPF_23_HZ` | 200 Hz / 23 Hz | 0x04 |
| `IM3J_GY_ODR_100_LPF_12_HZ` | 100 Hz / 12 Hz | 0x05 |
| `IM3J_GY_ODR_200_LPF_64_HZ` | 200 Hz / 64 Hz | 0x06 |
| `IM3J_GY_ODR_100_LPF_32_HZ` | 100 Hz / 32 Hz | 0x07 |

* * *
### 加速度计量程（FS）

| 宏定义 | 量程  | DS_RVision 下拉框数值  |
| --- | --- | --- |
| `IM3J_FS_XL_3_G` | ±3 g | 0x00 |
| `(*)IM3J_FS_XL_6_G` | ±6 g | 0x01 |
| `IM3J_FS_XL_12_G` | ±12 g | 0x02 |
| `IM3J_FS_XL_24_G` | ±24 g | 0x03 |

### 加速度计 LPF

| 宏定义 | DS_RVision 下拉框数值  |
| --- | --- |
| `IM3J_XL_LPF_NORMAL` | 0x00 |
| `IM3J_XL_LPF_OSR2` | 0x01 |
| `(*)IM3J_XL_LPF_OSR4` | 0x02 |

* * *

### 加速度计 ODR 配置
| 宏定义 | ODR | DS_RVision 下拉框数值  |
| --- | --- | --- |
| `IM3J_XL_ODR_12_5_HZ` | 12.5 Hz | 0x00 |
| `IM3J_XL_ODR_25_HZ` | 25 Hz | 0x01 |
| `IM3J_XL_ODR_50_HZ` | 50 Hz | 0x02 |
| `IM3J_XL_ODR_100_HZ` | 100 Hz | 0x03 |
| `(*)IM3J_XL_ODR_200_HZ` | 200 Hz | 0x04 |
| `IM3J_XL_ODR_400_HZ` | 400 Hz | 0x05 |
| `IM3J_XL_ODR_800_HZ` | 800 Hz | 0x06 |
| `IM3J_XL_ODR_1600_HZ` | 1600 Hz | 0x07 |

* * *
