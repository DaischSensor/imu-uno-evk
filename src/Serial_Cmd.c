#include <stdio.h>
#include <stdbool.h>
#include "zephyr_async_uart.h"
#include "DsImuSdk.h"
#include "imu_config.h"
#include "uart_protocol.h"
#include <zephyr/kernel.h>
#include <zephyr/sys/reboot.h>
#include "nvs_config.h"

static void Config_SaveToNvs(void);
static void Config_SaveRstCfgToNvs(void);

static uint8_t hexcheck(char recvnum)
{
    if (((recvnum >= '0') && (recvnum <= '9')) || ((recvnum >= 'a') && (recvnum <= 'z')) || ((recvnum >= 'A') && (recvnum <= 'Z')))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

// Converts char to hex value int
static uint8_t Char2Hex(unsigned char c, bool* pbOk)
{
    uint8_t u8Result = 0U;
    *pbOk = true;
    if (c >= '0' && c <= '9')
    {
        u8Result = c - '0';
    }
    else if (c >= 'A' && c <= 'F')
    {
        u8Result = c - 'A' + 10U;
    }
    else if (c >= 'a' && c <= 'f')
    {
        u8Result = c - 'a' + 10U;
    }
    else
    {
        *pbOk = false;
    }
    return u8Result;
}

// Converts (two-char) hex string to hex value int
static uint8_t Str2Hex(const unsigned char* pStr)
{
    bool bOk = true;
    uint8_t temp = 0U;
    uint8_t result = 0U;
    while (bOk)
    {
        result = result << 4U | temp;
        temp = Char2Hex(*(pStr++), &bOk);
    }
    return result;
}

static bool Config_ValidateUsrCfgAddrOfs(uint8_t addr)
{
    if (addr <= SERIAL_CMD_ADD_MAX)
        return true;
    else
        return false;
}

uint8_t Config_GetUsrCfg(uint8_t addr)
{
    uint8_t val = 0;
    switch (addr)
    {
        case 0x00: // UART output configuration
        {
            uint32_t baud = GetUartBaud();
            uint32_t u32UartSendPeriodUs = GetUartSendPeriodUs();
            uint8_t freq = 0;

            switch (baud)
            {
                case 115200: baud = 0x01; break;
                case 230400: baud = 0x02; break;
                case 460800: baud = 0x03; break;
                case 921600: baud = 0x04; break;
                default:     return 0xFF;
            }

            switch (u32UartSendPeriodUs)
            {
                case 100000: freq = 0x01; break; // 10Hz
                case 50000:  freq = 0x02; break; // 20Hz
                case 20000:  freq = 0x03; break; // 50Hz
                case 10000:  freq = 0x04; break; // 100Hz
                case 5000:   freq = 0x05; break; // 200Hz
                case 2000:   freq = 0x06; break; // 500Hz
                default:     freq = 0x04; break; // default 100Hz
            }

            val = (freq << 3) | (baud & 0x07);
            return val;
        }
#if defined(IMU_TARGET_IM3J)
        case 0x01: // Gyroscope configuration
        {
            uint8_t fs_val = 0;
            uint8_t lpf_val = 0;

            switch (CfgInfo.gyr_fs)
            {
                case IM3J_FS_GY_125_DPS:  fs_val = 0x00; break;
                case IM3J_FS_GY_250_DPS:  fs_val = 0x01; break;
                case IM3J_FS_GY_500_DPS:  fs_val = 0x02; break;
                case IM3J_FS_GY_1000_DPS: fs_val = 0x03; break;
                case IM3J_FS_GY_2000_DPS: fs_val = 0x04; break;
                default:             return 0xFF;
            }
            switch (CfgInfo.gyr_lpf)
            {
                case IM3J_GY_ODR_2000_LPF_532_HZ: lpf_val = 0x00; break;
                case IM3J_GY_ODR_2000_LPF_230_HZ: lpf_val = 0x01; break;
                case IM3J_GY_ODR_1000_LPF_116_HZ: lpf_val = 0x02; break;
                case IM3J_GY_ODR_400_LPF_47_HZ:   lpf_val = 0x03; break;
                case IM3J_GY_ODR_200_LPF_23_HZ:   lpf_val = 0x04; break;
                case IM3J_GY_ODR_100_LPF_12_HZ:   lpf_val = 0x05; break;
                case IM3J_GY_ODR_200_LPF_64_HZ:   lpf_val = 0x06; break;
                case IM3J_GY_ODR_100_LPF_32_HZ:   lpf_val = 0x07; break;
                default:                     return 0xFF;
            }
            return (lpf_val << 4) | (fs_val & 0x0F);
        }

        case 0x02: // Accelerometer configuration
        {
            uint8_t fs_val = 0;
            uint8_t lpf_val = 0;

            switch (CfgInfo.acc_fs)
            {
                case IM3J_FS_XL_3_G:  fs_val = 0x00; break;
                case IM3J_FS_XL_6_G:  fs_val = 0x01; break;
                case IM3J_FS_XL_12_G: fs_val = 0x02; break;
                case IM3J_FS_XL_24_G: fs_val = 0x03; break;
                default:         return 0xFF;
            }
            switch (CfgInfo.acc_lpf)
            {
                case IM3J_XL_LPF_NORMAL: lpf_val = 0x00; break;
                case IM3J_XL_LPF_OSR2:   lpf_val = 0x01; break;
                case IM3J_XL_LPF_OSR4:   lpf_val = 0x02; break;
                default:                 return 0xFF;
            }
            return (lpf_val << 4) | (fs_val & 0x0F);
        }

        case 0x03:
        {
            uint8_t odr_val = 0;

            switch (CfgInfo.acc_odr)
            {
                case IM3J_XL_ODR_12_5_HZ:   odr_val = 0x00; break;
                case IM3J_XL_ODR_25_HZ:     odr_val = 0x01; break;
                case IM3J_XL_ODR_50_HZ:     odr_val = 0x02; break;
                case IM3J_XL_ODR_100_HZ:    odr_val = 0x03; break;
                case IM3J_XL_ODR_200_HZ:    odr_val = 0x04; break;
                case IM3J_XL_ODR_400_HZ:    odr_val = 0x05; break;
                case IM3J_XL_ODR_800_HZ:    odr_val = 0x06; break;
                case IM3J_XL_ODR_1600_HZ:   odr_val = 0x07; break;
                default:         return 0xFF;
            }
            return (odr_val & 0x0F);
        }

#elif defined(IMU_TARGET_IM8S)
        case 0x01: // Gyroscope configuration
        {
            uint8_t fs_val = 0;
            uint8_t lpf_val = 0;

            switch (CfgInfo.gyr_fs)
            {
                case IM8S_FS_GY_125_DPS:  fs_val = 0x00; break;
                case IM8S_FS_GY_250_DPS:  fs_val = 0x01; break;
                case IM8S_FS_GY_500_DPS:  fs_val = 0x02; break;
                case IM8S_FS_GY_1000_DPS: fs_val = 0x03; break;
                case IM8S_FS_GY_2000_DPS: fs_val = 0x04; break;
                default:             return 0xFF;
            }
            switch (CfgInfo.gyr_lpf)
            {
                case IM8S_GY_LPF_274HZ:   lpf_val = 0x00; break;
                case IM8S_GY_LPF_212HZ:   lpf_val = 0x01; break;
                case IM8S_GY_LPF_150HZ:   lpf_val = 0x02; break;
                case IM8S_GY_LPF_390HZ:   lpf_val = 0x03; break;
                case IM8S_GY_LPF_99HZ:    lpf_val = 0x04; break;
                case IM8S_GY_LPF_50HZ:    lpf_val = 0x05; break;
                case IM8S_GY_LPF_25HZ:    lpf_val = 0x06; break;
                case IM8S_GY_LPF_12HZ:    lpf_val = 0x07; break;
                case IM8S_GY_LPF_DISABLE: lpf_val = 0x08; break;
                default:             return 0xFF;
            }
            return (lpf_val << 4) | (fs_val & 0x0F);
        }

        case 0x02: // Accelerometer configuration
        {
            uint8_t fs_val = 0;
            uint8_t lpf_val = 0;

            switch (CfgInfo.acc_fs)
            {
                case IM8S_FS_XL_2_G:  fs_val = 0x00; break;
                case IM8S_FS_XL_4_G:  fs_val = 0x01; break;
                case IM8S_FS_XL_8_G:  fs_val = 0x02; break;
                case IM8S_FS_XL_16_G: fs_val = 0x03; break;
                default:         return 0xFF;
            }
            switch (CfgInfo.acc_lpf)
            {
                case IM8S_XL_LPF_417HZ:   lpf_val = 0x00; break;
                case IM8S_XL_LPF_167HZ:   lpf_val = 0x01; break;
                case IM8S_XL_LPF_83HZ:    lpf_val = 0x02; break;
                case IM8S_XL_LPF_37HZ:    lpf_val = 0x03; break;
                case IM8S_XL_LPF_16HZ:    lpf_val = 0x04; break;
                case IM8S_XL_LPF_8HZ:     lpf_val = 0x05; break;
                case IM8S_XL_LPF_4HZ:     lpf_val = 0x06; break;
                case IM8S_XL_LPF_2HZ:     lpf_val = 0x07; break;
                case IM8S_XL_LPF_DISABLE: lpf_val = 0x08; break;
                default:             return 0xFF;
            }
            return (lpf_val << 4) | (fs_val & 0x0F);
        }

        case 0x03:
        {
            uint8_t odr_val = 0;

            switch (CfgInfo.acc_odr)
            {
                case BYPASS:                odr_val = 0x08; break;
                default:         return 0xFF;
            }
            return (odr_val & 0x0F);
        }
#endif
        default:
        {
            return 0xFF;
        }
    }
}

uint8_t Config_SetUsrCfg(uint8_t addr, uint8_t val)
{
    switch (addr)
    {
        case 0x00: // UART output configuration
        {
            // SendReadWriteAck(addr, val);
            // k_msleep(1000);
            uint8_t baud = val & 0x07;
            uint32_t u32CurrentBaud = GetUartCurrentBaud();
            uint32_t u32NewBaud = 0;
            uint8_t freq = (val >> 3) & 0x0F;

            switch (baud) {
                case 0x01: u32NewBaud = 115200U; break;
                case 0x02: u32NewBaud = 230400U; break;
                case 0x03: u32NewBaud = 460800U; break;
                case 0x04: u32NewBaud = 921600U; break;
                default:   return 0xFF;
            }
            // Save configuration value
            setUartBaud(u32NewBaud);

            // UART output frequency
            switch (freq)
            {
                case 0x01: setUartSendPeriodUs(100000U); break;
                case 0x02: setUartSendPeriodUs(50000U);  break;
                case 0x03: setUartSendPeriodUs(20000U);  break;
                case 0x04: setUartSendPeriodUs(10000U);  break;
                case 0x05: setUartSendPeriodUs(5000U);   break;
                case 0x06: setUartSendPeriodUs(2000U);   break;
                default:   return 0xFF;
            }

            if (u32NewBaud != u32CurrentBaud)
            {
                gImuSerialOutputEnabled = false;
                UartReconfig(u32NewBaud);
                SendReadWriteAck(addr, val);
            }
            else
            {
                SendReadWriteAck(addr, val);
            }

            return val;
        }
#if defined(IMU_TARGET_IM3J)
        case 0x01: // Gyroscope configuration
        {
            uint8_t fs = val & 0x0F;
            uint8_t lpf = (val >> 4) & 0x0F;

            // Full scale
            switch (fs)
            {
                case 0x00: CfgInfo.gyr_fs = IM3J_FS_GY_125_DPS;  break;
                case 0x01: CfgInfo.gyr_fs = IM3J_FS_GY_250_DPS;  break;
                case 0x02: CfgInfo.gyr_fs = IM3J_FS_GY_500_DPS;  break;
                case 0x03: CfgInfo.gyr_fs = IM3J_FS_GY_1000_DPS; break;
                case 0x04: CfgInfo.gyr_fs = IM3J_FS_GY_2000_DPS; break;
                default:   return 0xFF;
            }

            // ODR / Low-pass filter
            switch (lpf)
            {
                case 0x00: CfgInfo.gyr_lpf = IM3J_GY_ODR_2000_LPF_532_HZ; break;
                case 0x01: CfgInfo.gyr_lpf = IM3J_GY_ODR_2000_LPF_230_HZ; break;
                case 0x02: CfgInfo.gyr_lpf = IM3J_GY_ODR_1000_LPF_116_HZ; break;
                case 0x03: CfgInfo.gyr_lpf = IM3J_GY_ODR_400_LPF_47_HZ;   break;
                case 0x04: CfgInfo.gyr_lpf = IM3J_GY_ODR_200_LPF_23_HZ;   break;
                case 0x05: CfgInfo.gyr_lpf = IM3J_GY_ODR_100_LPF_12_HZ;   break;
                case 0x06: CfgInfo.gyr_lpf = IM3J_GY_ODR_200_LPF_64_HZ;   break;
                case 0x07: CfgInfo.gyr_lpf = IM3J_GY_ODR_100_LPF_32_HZ;   break;
                default:   return 0xFF;
            }

            return val;
        }

        case 0x02: // Accelerometer configuration
        {
            uint8_t fs = val & 0x0F;
            uint8_t lpf = (val >> 4) & 0x0F;

            switch (fs)
            {
                case 0x00: CfgInfo.acc_fs = IM3J_FS_XL_3_G;  break;
                case 0x01: CfgInfo.acc_fs = IM3J_FS_XL_6_G;  break;
                case 0x02: CfgInfo.acc_fs = IM3J_FS_XL_12_G; break;
                case 0x03: CfgInfo.acc_fs = IM3J_FS_XL_24_G; break;
                default:   return 0xFF;
            }
            switch (lpf)
            {
                case 0x00: CfgInfo.acc_lpf = IM3J_XL_LPF_NORMAL; break;
                case 0x01: CfgInfo.acc_lpf = IM3J_XL_LPF_OSR2;   break;
                case 0x02: CfgInfo.acc_lpf = IM3J_XL_LPF_OSR4;   break;
                default:   return 0xFF;
            }

            return val;
        }

        case 0x03:
        {
            uint8_t odr = val & 0x0F;

            // Output data rate
            switch (odr)
            {
                case 0x00: CfgInfo.acc_odr = IM3J_XL_ODR_12_5_HZ;  break;
                case 0x01: CfgInfo.acc_odr = IM3J_XL_ODR_25_HZ;  break;
                case 0x02: CfgInfo.acc_odr = IM3J_XL_ODR_50_HZ; break;
                case 0x03: CfgInfo.acc_odr = IM3J_XL_ODR_100_HZ; break;
                case 0x04: CfgInfo.acc_odr = IM3J_XL_ODR_200_HZ;  break;
                case 0x05: CfgInfo.acc_odr = IM3J_XL_ODR_400_HZ;  break;
                case 0x06: CfgInfo.acc_odr = IM3J_XL_ODR_800_HZ; break;
                case 0x07: CfgInfo.acc_odr = IM3J_XL_ODR_1600_HZ; break;
                default:   return 0xFF;
            }
            return val;
        }

#elif defined(IMU_TARGET_IM8S)

        case 0x01: // Gyroscope configuration
        {
            uint8_t fs = val & 0x0F;
            uint8_t lpf = (val >> 4) & 0x0F;

            // Full scale
            switch (fs)
            {
                case 0x00: CfgInfo.gyr_fs = IM8S_FS_GY_125_DPS;  break;
                case 0x01: CfgInfo.gyr_fs = IM8S_FS_GY_250_DPS;  break;
                case 0x02: CfgInfo.gyr_fs = IM8S_FS_GY_500_DPS;  break;
                case 0x03: CfgInfo.gyr_fs = IM8S_FS_GY_1000_DPS; break;
                case 0x04: CfgInfo.gyr_fs = IM8S_FS_GY_2000_DPS; break;
                default:   return 0xFF;
            }

            // Low-pass filter
            switch (lpf)
            {
                case 0x00: CfgInfo.gyr_lpf = IM8S_GY_LPF_274HZ; break;
                case 0x01: CfgInfo.gyr_lpf = IM8S_GY_LPF_212HZ; break;
                case 0x02: CfgInfo.gyr_lpf = IM8S_GY_LPF_150HZ; break;
                case 0x03: CfgInfo.gyr_lpf = IM8S_GY_LPF_390HZ; break;
                case 0x04: CfgInfo.gyr_lpf = IM8S_GY_LPF_99HZ;  break;
                case 0x05: CfgInfo.gyr_lpf = IM8S_GY_LPF_50HZ;  break;
                case 0x06: CfgInfo.gyr_lpf = IM8S_GY_LPF_25HZ;  break;
                case 0x07: CfgInfo.gyr_lpf = IM8S_GY_LPF_12HZ;  break;
                case 0x08: CfgInfo.gyr_lpf = IM8S_GY_LPF_DISABLE; break;
                default:   return 0xFF;
            }

            return val;
        }

        case 0x02: // Accelerometer configuration (filter & full scale)
        {
            uint8_t fs = val & 0x0F;
            uint8_t lpf = (val >> 4) & 0x0F;

            // Full scale
            switch (fs)
            {
                case 0x00: CfgInfo.acc_fs = IM8S_FS_XL_2_G;  break;
                case 0x01: CfgInfo.acc_fs = IM8S_FS_XL_4_G;  break;
                case 0x02: CfgInfo.acc_fs = IM8S_FS_XL_8_G;  break;
                case 0x03: CfgInfo.acc_fs = IM8S_FS_XL_16_G; break;
                default:   return 0xFF;
            }

            // Filter
            switch (lpf)
            {
                case 0x00: CfgInfo.acc_lpf = IM8S_XL_LPF_417HZ; break;
                case 0x01: CfgInfo.acc_lpf = IM8S_XL_LPF_167HZ; break;
                case 0x02: CfgInfo.acc_lpf = IM8S_XL_LPF_83HZ;  break;
                case 0x03: CfgInfo.acc_lpf = IM8S_XL_LPF_37HZ;  break;
                case 0x04: CfgInfo.acc_lpf = IM8S_XL_LPF_16HZ;  break;
                case 0x05: CfgInfo.acc_lpf = IM8S_XL_LPF_8HZ;   break;
                case 0x06: CfgInfo.acc_lpf = IM8S_XL_LPF_4HZ;   break;
                case 0x07: CfgInfo.acc_lpf = IM8S_XL_LPF_2HZ;   break;
                case 0x08: CfgInfo.acc_lpf = IM8S_XL_LPF_DISABLE; break;
                default:   return 0xFF;
            }

            return val;
        }

        case 0x03:
        {
            uint8_t odr = val & 0x0F;

            // Output data rate
            switch (odr)
            {
                case 0x08: CfgInfo.acc_odr = BYPASS;  break;
                default:   return 0xFF;
            }
            return val;
        }

#endif
        default:
            return 0xFF;
    }
}

bool Config_ResetParameters(void)
{
#if defined(IMU_TARGET_IM3J)
    CfgInfo.acc_fs = IM3J_FS_XL_6_G;
    CfgInfo.gyr_fs = IM3J_FS_GY_1000_DPS;

    CfgInfo.acc_lpf = IM3J_XL_LPF_OSR4;
    CfgInfo.gyr_lpf = IM3J_GY_ODR_200_LPF_23_HZ;

    CfgInfo.acc_odr = IM3J_XL_ODR_200_HZ;


#elif defined(IMU_TARGET_IM8S)
    CfgInfo.acc_fs = IM8S_FS_XL_8_G;
    CfgInfo.gyr_fs = IM8S_FS_GY_250_DPS;

    CfgInfo.acc_lpf = IM8S_XL_LPF_16HZ;
    CfgInfo.gyr_lpf = IM8S_GY_LPF_50HZ;

    CfgInfo.acc_odr = BYPASS;

#endif

    return true;
}

/**************************************************************************
 * Helper function: Check validity of SET command and extract address and data
 * Parameters:
 *   pBuffer: command buffer, format like "$DSIMC,SET,<addr><...>,<regVal>..."
 *   rxBytes: number of bytes in buffer
 *   pAddrOfs: output, extracted address offset
 *   pRegVal:  output, extracted register value
 * Return:
 *   true / false
 **************************************************************************/
bool CheckSetCmdValidity(const uint8_t* pBuffer, uint32_t rxBytes,
    uint8_t* pAddrOfs, uint8_t* pRegVal)
{
    if (hexcheck(pBuffer[11]) == 0)
    {
        SendOnlyNAck();
        return false;
    }
    *pAddrOfs = Str2Hex(&pBuffer[11]);
    if ((*pAddrOfs == SERIAL_CMD_ADD_ERR) || (*pAddrOfs > SERIAL_CMD_ADD_MAX))
    {
        SendERRWriteAck(*pAddrOfs, 1);
        return false;
    }
    uint8_t i = 11U;
    while (i < rxBytes && pBuffer[i] != ',')
    {
        i++;
    }
    if (i >= rxBytes)
    {
        SendOnlyNAck();
        return false;
    }
    *pRegVal = Str2Hex(&pBuffer[i + 1]);

    switch (*pAddrOfs)
    {
        case 0:
            if (((*pRegVal & 0x07) > 4) || ((*pRegVal & 0x07) == 0) || (((*pRegVal) & 0xF8) > 0x30))
            {
                SendERRWriteAck(*pAddrOfs, 2);
                return false;
            }
            break;
        case 1:
            if (((*pRegVal & 0x0F) > 5) || (((*pRegVal) & 0xF0) > 0x80))
            {
                SendERRWriteAck(*pAddrOfs, 2);
                return false;
            }
            break;
        case 2:
            if ((*pRegVal & 0x0F) > 3)
            {
                SendERRWriteAck(*pAddrOfs, 2);
                return false;
            }
            break;
        default:
            break;
    }
    return true;
}

static void Config_SaveToNvs(void)
{
    StoredConfig_t cfg;
    cfg.baud_rate = GetUartBaud();
    cfg.send_period_us = GetUartSendPeriodUs();
    cfg.acc_fs = CfgInfo.acc_fs;
    cfg.gyr_fs = CfgInfo.gyr_fs;
    cfg.acc_odr = CfgInfo.acc_odr;
    cfg.acc_lpf = CfgInfo.acc_lpf;
    cfg.gyr_lpf = CfgInfo.gyr_lpf;

    NvsConfigSave(&cfg);
}

static void Config_SaveRstCfgToNvs(void)
{
    StoredConfig_t cfg;
    cfg.baud_rate = 921600;
    cfg.send_period_us = 10000U;
    setUartSendPeriodUs(10000U);
    cfg.acc_fs = CfgInfo.acc_fs;
    cfg.gyr_fs = CfgInfo.gyr_fs;
    cfg.acc_odr = CfgInfo.acc_odr;
    cfg.acc_lpf = CfgInfo.acc_lpf;
    cfg.gyr_lpf = CfgInfo.gyr_lpf;

    NvsConfigSave(&cfg);
}

bool ProcessDSIMCCommand(uint8_t* pBuffer, uint32_t rxBytes)
{
    uint8_t crc_ack;
    bool cmdHandled = true;
    uint8_t u8AddrOfs, u8RegVal;

    if (memcmp(&pBuffer[7], "GET,", 4) == 0U)
    {
        if (hexcheck(pBuffer[11]) == 0)
        {
            SendOnlyNAck();
            return false;
        }
        u8AddrOfs = Str2Hex(&pBuffer[11]);
        if ((u8AddrOfs == SERIAL_CMD_ADD_ERR) || (u8AddrOfs > SERIAL_CMD_ADD_MAX))
        {
            SendOnlyNAck();
            return false;
        }
        if (Config_ValidateUsrCfgAddrOfs(u8AddrOfs))
        {
            SendReadWriteAck(u8AddrOfs, Config_GetUsrCfg(u8AddrOfs));
            // SerialSendMsg(pBuffer, 15);
        }
    }
    else if (memcmp(&pBuffer[7], "SET,", 4) == 0U)
    {
        if (!CheckSetCmdValidity(pBuffer, rxBytes, &u8AddrOfs, &u8RegVal))
        {
            return false;
        }

        if (Config_ValidateUsrCfgAddrOfs(u8AddrOfs))
        {
            if (Config_SetUsrCfg(u8AddrOfs, u8RegVal) == u8RegVal)
            {
                Config_SaveToNvs();    /* Write to NVS */
                SendReadWriteAck(u8AddrOfs, u8RegVal);
                if (u8AddrOfs >= 0x01 && u8AddrOfs <= 0x03)
                    gImuReinitFlag = true;
            }
            else
            {
                SendERRWriteAck(u8AddrOfs, 0);
            }
        }
        else
        {
            SendERRWriteAck(u8AddrOfs, 1);
        }
    }
    else if (memcmp(&pBuffer[7], "RST", 3) == 0U)
    {
        if (Config_ResetParameters())
        {
            Config_ResetParameters();    /* Write factory defaults to NVS */
            Config_SaveRstCfgToNvs();
            SendAck();
            gImuReinitFlag = true;

            // Config_SaveToNvs();    /* Write factory defaults to NVS */
            // SendAck();
        }
    }
    else if (memcmp(&pBuffer[7], "SN", 2) == 0U)
    {
        uint8_t sn[10];
        static DsImu_InfoType xInfoGetSN;
        uint8_t ret = DsImu_GetInfo(&xInfoGetSN);

        if (ret == 0U) {
            uint8_t copySize = (sizeof(xInfoGetSN.sn) < 10) ? sizeof(xInfoGetSN.sn) : 10;
            memcpy(sn, xInfoGetSN.sn, copySize);

            for (uint8_t i = copySize; i < 10; i++) {
                sn[i] = 0;
            }
        }
        else {
            memset(sn, 0, 10);
        }

        uint16_t sn_last = ((uint16_t)sn[3] << 8) | sn[4];

        uint8_t snLen = (uint8_t)sprintf((char*)Serial_ucAckBuffer,
            "$DSIMC,ACK,%02u%02u%02u%04u*",
            sn[0], sn[1], sn[2], sn_last);

        crc_ack = CRC_8(Serial_ucAckBuffer, snLen - 1); /* Exclude '*' */
        Serial_ucAckBuffer[snLen] = Hex2Char((crc_ack >> 4), NULL);
        Serial_ucAckBuffer[snLen + 1] = Hex2Char((crc_ack & 0x0F), NULL);
        sprintf((char*)&Serial_ucAckBuffer[snLen + 2], "\r\n");
        SerialSendMsg(Serial_ucAckBuffer, strlen((char*)Serial_ucAckBuffer));
    }
    else if (memcmp(&pBuffer[7], "VER", 3) == 0U)
    {
        static DsImu_InfoType xInfoGetVER;
        uint8_t ret = DsImu_GetInfo(&xInfoGetVER);
        char verStr[16] = { 0 };

        if (ret == 0U) {
            const char* p = xInfoGetVER.version;
            uint8_t underscoreCount = 0;
            const char* start = NULL;

            while (*p != '\0') {
                if (*p == '_') {
                    underscoreCount++;
                    if (underscoreCount == 3U) {
                        start = p + 1;
                    }
                }
                else if (((*p == '-') || (*p == '@')) && (start != NULL)) {
                    uint8_t len = (uint8_t)(p - start);
                    if (len >= sizeof(verStr)) {
                        len = (uint8_t)(sizeof(verStr) - 1U);
                    }
                    memcpy(verStr, start, len);
                    verStr[len] = '\0';
                    break;
                }
                p++;
            }
        }
        if (verStr[0] == '\0') {
            strncpy(verStr, "V0.0.0", sizeof(verStr) - 1);
        }

        const char* verNum = (verStr[0] == 'V') ? &verStr[1] : verStr;

        uint8_t verLen = (uint8_t)sprintf((char*)Serial_ucAckBuffer,
            "$DSIMC,ACK,%s*",
            verNum);

        crc_ack = CRC_8(Serial_ucAckBuffer, verLen - 1); /* Exclude '*' */
        Serial_ucAckBuffer[verLen] = Hex2Char((crc_ack >> 4), NULL);
        Serial_ucAckBuffer[verLen + 1] = Hex2Char((crc_ack & 0x0F), NULL);
        sprintf((char*)&Serial_ucAckBuffer[verLen + 2], "\r\n");
        // printk(xInfoGetVER.version);
        SerialSendMsg(Serial_ucAckBuffer, strlen((char*)Serial_ucAckBuffer));
    }
    else if (memcmp(&pBuffer[7], "RBT", 3) == 0U)
    {
        // SendAck();
        // sys_reboot(SYS_REBOOT_COLD); // Cold reboot
    }
    else if (memcmp(&pBuffer[7], "STP", 3) == 0U)
    {
        gImuSerialOutputEnabled = false;
    }
    else if (memcmp(&pBuffer[7], "RES", 3) == 0U)
    {
        gImuSerialOutputEnabled = true;
    }
    else
    {
        SendOnlyNAck();
        cmdHandled = false;
    }
    return cmdHandled;
}

/**************************************************************************
 * Unified processing of UART received data
 * Parameters:
 *   pBuffer: pointer to the command buffer for this port
 *   pRxBytes: pointer to the received byte count (cleared after processing)
 * Return:
 *   true/false
 **************************************************************************/
bool ProcessSerialBuffer(uint8_t* pBuffer, uint32_t* pRxBytes)
{
    bool bCmdRecv = false;
    if (*pRxBytes > 10U &&
        (pBuffer[*pRxBytes - 2U] == '\r' || pBuffer[*pRxBytes - 2U] == '\n'))
    {
        // Check for "$DSIMC," command
        if (memcmp(pBuffer, "$DSIMC,", 7) == 0U)
        {
            bCmdRecv = ProcessDSIMCCommand(pBuffer, *pRxBytes);
        }
        // Clear buffer, prepare for next reception
        *pRxBytes = 0;
        memset(pBuffer, 0, SERIAL_CMD_BUFF_SIZE);
    }
    return bCmdRecv;
}