#ifndef NVS_CONFIG_H
#define NVS_CONFIG_H

#include <stdint.h>
#include <stdbool.h>

#define NVS_CONFIG_MAGIC    0xDA15C401   /* "UIMU" */

#if defined(IMU_TARGET_IM3J)
#define NVS_ID_CONFIG       0x0001       /* Configuration of IM3J*/
#elif defined(IMU_TARGET_IM8S)
#define NVS_ID_CONFIG       0x0002       /* Configuration of IM8S*/
#else
#define NVS_ID_CONFIG       0x0001       /* default */
#endif

typedef struct __attribute__((packed)) {
    uint32_t magic;           /* Magic number, used to determine whether it has been written before */
    uint32_t baud_rate;       /* gUartDataBaud */
    uint32_t send_period_us;  /* gUartSendPeriodUs */
    uint8_t  acc_fs;          /* CfgInfo.acc_fs */
    uint8_t  gyr_fs;          /* CfgInfo.gyr_fs */
    uint8_t  acc_odr;         /* CfgInfo.acc_odr */
    uint8_t  acc_lpf;         /* CfgInfo.acc_lpf */
    uint8_t  gyr_lpf;         /* CfgInfo.gyr_lpf */
    uint8_t  crc;             /* CRC */
    uint8_t  reserved[2];     /* Align and pad */
} StoredConfig_t;

/**
 * @brief Initialize NVS (mount partition)
 * @return 0 on success, negative on failure
 */
int NvsConfigInit(void);

/**
 * @brief Load configuration from NVS
 * @param cfg Output parameter, loaded into this structure
 * @return 0=success, 1=no stored data (first boot), negative=error
 */
int NvsConfigLoad(StoredConfig_t* cfg);

/**
 * @brief Save configuration to NVS
 * @param cfg Configuration to be saved
 * @return 0 on success, negative on failure
 */
int NvsConfigSave(const StoredConfig_t* cfg);

/**
 * @brief Erase configuration in NVS (for factory reset)
 * @return 0 on success, negative on failure
 */
int NvsConfigErase(void);

#endif /* NVS_CONFIG_H */
