<div align="right">

**English** | [简体中文](../zh-CN/IM3J_Configuration_Mapping_Guide.zh-CN.md)

</div>

# IM3J Configuration Mapping Guide

> Items marked with an asterisk (*) before the macro definition are default items.

### Gyroscope Full Scale Range

| Macro Definition | Range | DS_RVision Dropdown Value |
| --- | --- | --- |
| `IM3J_FS_GY_125_DPS` | ±125 dps | 0x00 |
| `IM3J_FS_GY_250_DPS` | ±250 dps | 0x01 |
| `IM3J_FS_GY_500_DPS` | ±500 dps | 0x02 |
| `(*)IM3J_FS_GY_1000_DPS` | ±1000 dps | 0x03 |
| `IM3J_FS_GY_2000_DPS` | ±2000 dps | 0x04 |

### Gyroscope LPF / ODR

| Macro Definition | Configuration | DS_RVision Dropdown Value |
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
### Accelerometer Full Scale Range

| Macro Definition | Range | DS_RVision Dropdown Value |
| --- | --- | --- |
| `IM3J_FS_XL_3_G` | ±3 g | 0x00 |
| `(*)IM3J_FS_XL_6_G` | ±6 g | 0x01 |
| `IM3J_FS_XL_12_G` | ±12 g | 0x02 |
| `IM3J_FS_XL_24_G` | ±24 g | 0x03 |

### Accelerometer LPF

| Macro Definition |  DS_RVision Dropdown Value |
| --- | --- |
| `IM3J_XL_LPF_NORMAL` | 0x00 |
| `IM3J_XL_LPF_OSR2` | 0x01 |
| `(*)IM3J_XL_LPF_OSR4` | 0x02 |

* * *

### Accelerometer ODR Configuration

| Macro Definition | ODR | DS_RVision Dropdown Value |
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