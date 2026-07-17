/**
 * @file    basic_output_example.c
 * @brief   Basic six-axis data output example
 *
 * [Function]
 *   Demonstrates how to use DsImuSdk to obtain IMU six-axis data
 *   (acceleration + angular rate) and pack it into binary frames for
 *   serial output.
 *
 *   Does not output attitude / quaternion.
 *
 * [SDK API Call Flow]
 *
 *   ┌─────────────────────────────────────────────────────────────────┐
 *   │ 1. Fill DsImu_CfgInfoType (range/rate/HAL function pointers)    │
 *   │ 2. DsImu_Init(&CfgInfo)        → Initialize SDK                 │
 *   │ 3. DsImu_GetData(&cor, &raw)   → Read data in a loop            │
 *   │ 4. Composer_PrintImu_Basic()   → Assemble binary frame          │
 *   └─────────────────────────────────────────────────────────────────┘
 *
 * [Binary Frame Format (72 bytes)]
 *   D0-D2:   Frame header 0xA5 0x5A 0x00
 *   D3-D4:   Command word 0x10 0x40
 *   D5:      Frame counter (0~255)
 *   D6-D13:  Timestamp (UInt64, ms, LE)
 *   D14-D25: Acceleration X/Y/Z (Float×3, m/s²)
 *   D26-D37: Angular rate X/Y/Z (Float×3, °/s)
 *   D38-D68: ★ This example fills 0 (attitude/quaternion/temperature/status field) ★
 *   D69:     CRC-8
 *   D70-D71: Frame tail 0x0D 0x0A
 */

 /* ========================================================================
  *  0. Header Files
  * ======================================================================== */
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/drivers/gpio.h>
#include <string.h>
#include <stdint.h>

#include "DsImuSdk.h"   /* ★ The only header file required by the SDK ★ */


  /* ========================================================================
   *  1. HAL Layer Implementation (implemented by user according to hardware)
   * ========================================================================
   *  The SDK accesses hardware via 5 function pointers in DsImu_CfgInfoType:
   *
   *   .transfer    — SPI read/write
   *   .cancel      — Cancel SPI transfer
   *   .getStatus   — Get transfer result
   *   .systick     — Get microsecond timestamp
   *   + cs_select / cs_deselect (called inside spiTransfer)
   *
   *  The following is a reference implementation for the Zephyr platform.
   */

static const struct device* g_spi_dev;
static const struct device* g_gpio_dev;
static struct spi_config g_spi_cfg = {
    .frequency = 1000000,
    .operation = SPI_OP_MODE_MASTER | SPI_TRANSFER_MSB | SPI_WORD_SET(8),
    .slave = 0, .cs = NULL,
};

/* Chip select */
static void cs_select(DsImuChipId_t eChipId)
{
#if defined(IMU_TARGET_IM3J)
    switch (eChipId) {
        case DS_IM3J_ACC:  gpio_pin_set(g_gpio_dev, 9, 0); break;
        case DS_IM3J_GYR:  gpio_pin_set(g_gpio_dev, 8, 0); break;
        case DS_EEPROM:    gpio_pin_set(g_gpio_dev, 4, 0); break;
        default: break;
    }
#elif defined(IMU_TARGET_IM8S)
    switch (eChipId) {
        case DS_IM8S_IMU:  gpio_pin_set(g_gpio_dev, 9, 0); break;
        case DS_EEPROM:    gpio_pin_set(g_gpio_dev, 4, 0); break;
        default: break;
    }
#endif
}

static void cs_deselect(DsImuChipId_t eChipId)
{
#if defined(IMU_TARGET_IM3J)
    switch (eChipId) {
        case DS_IM3J_ACC:  gpio_pin_set(g_gpio_dev, 9, 1); break;
        case DS_IM3J_GYR:  gpio_pin_set(g_gpio_dev, 8, 1); break;
        case DS_EEPROM:    gpio_pin_set(g_gpio_dev, 4, 1); break;
        default: break;
    }
#elif defined(IMU_TARGET_IM8S)
    switch (eChipId) {
        case DS_IM8S_IMU:  gpio_pin_set(g_gpio_dev, 9, 1); break;
        case DS_EEPROM:    gpio_pin_set(g_gpio_dev, 4, 1); break;
        default: break;
    }
#endif
}

/* ★ HAL Interface 1/5: SPI Transfer */
uint8_t spiTransfer(DsImuChipId_t eChipId,
    DsImu_SpiBuffType* pxSpiBufArray,
    uint8_t u8ArraySize)
{
    if (!pxSpiBufArray || u8ArraySize == 0) return 1;
    cs_select(eChipId);
    k_usleep(10);
    for (uint8_t i = 0; i < u8ArraySize; i++) {
        uint16_t len = pxSpiBufArray[i].u16Len;
        if (len == 0 || len > 256) { cs_deselect(eChipId); return 1; }
        struct spi_buf tx = { .buf = pxSpiBufArray[i].pTxBuf, .len = len };
        struct spi_buf rx = { .buf = pxSpiBufArray[i].pRxBuf, .len = len };
        struct spi_buf_set tx_s = { .buffers = &tx, .count = 1 };
        struct spi_buf_set rx_s = { .buffers = &rx, .count = 1 };
        if (spi_transceive(g_spi_dev, &g_spi_cfg, &tx_s, &rx_s) != 0) {
            cs_deselect(eChipId); return 1;
        }
    }
    cs_deselect(eChipId);
    return 0;
}

/* ★ HAL Interface 2/5: Cancel Transfer */
void spiCancel(DsImuChipId_t eChipId) { cs_deselect(eChipId); }

/* ★ HAL Interface 3/5: Get Transfer Result */
uint8_t spiGetTransferResult(DsImuChipId_t eChipId) { (void)eChipId; return 0; }

/* ★ HAL Interface 4/5: Microsecond Timestamp */
static uint64_t g_cycles_per_sec;
void TickInit(void) { g_cycles_per_sec = sys_clock_hw_cycles_per_sec(); }
uint64_t GetTickUs(void)
{
    uint64_t c = k_cycle_get_64();
    return (c / g_cycles_per_sec) * 1000000ULL +
        (c % g_cycles_per_sec) * 1000000ULL / g_cycles_per_sec;
}


/* ========================================================================
 *  2. SDK Configuration (★ modify as needed ★)
 * ======================================================================== */
DsImu_CfgInfoType CfgInfo = {
#if defined(IMU_TARGET_IM3J)
    .module_type = MODULE_IM3J,
    .acc_fs = IM3J_FS_XL_6_G,              /* Accelerometer full scale: ±6g         */
    .gyr_fs = IM3J_FS_GY_1000_DPS,         /* Gyroscope full scale: ±1000°/s        */
    .acc_odr = IM3J_XL_ODR_200_HZ,          /* Accelerometer ODR: 200Hz             */
    .acc_lpf = IM3J_XL_LPF_OSR4,            /* Accelerometer LPF                    */
    .gyr_lpf = IM3J_GY_ODR_200_LPF_23_HZ,   /* Gyroscope ODR+LPF: 200Hz/23Hz        */
#elif defined(IMU_TARGET_IM8S)
    .module_type = MODULE_IM8S,
    .acc_fs = IM8S_FS_XL_8_G,              /* Accelerometer full scale: ±8g         */
    .gyr_fs = IM8S_FS_GY_250_DPS,          /* Gyroscope full scale: ±250°/s         */
    .acc_odr = BYPASS,
    .acc_lpf = IM8S_XL_LPF_16HZ,            /* Accelerometer LPF: 16Hz              */
    .gyr_lpf = IM8S_GY_LPF_50HZ,            /* Gyroscope LPF: 50Hz                  */
#endif
    .tim_freq = FREQ_500HZ,         /* Data acquisition frequency: 500Hz */
    .u64UsTickValueMax = 0xFFFFFFFFFFFFFFFFU,
    .mcu_freq = 160U,               /* MCU main frequency (MHz)          */
    .u32SpiTimeoutUs = {10000U, 10000U},   /* SPI timeout (us)            */

    /* ★ 5 HAL function pointers ★ */
    .transfer = spiTransfer,
    .cancel = spiCancel,
    .systick = GetTickUs,
    .getStatus = spiGetTransferResult,

    /* Mounting angles / external angles / position (unit: rad; can be set to 0 for basic output) */
    .fMntAng = {0.0F, 0.0F, 0.0F},
    .fExtAng = {0.0F, 0.0F, 0.0F},
    .fPos = {0.0F, 0.0F, 0.0F},
};


/* ========================================================================
 *  3. Binary Protocol Packing (only six axes)
 * ======================================================================== */

 /* CRC-8 */
static uint8_t CRC_8(const uint8_t* buf, uint16_t len)
{
    uint8_t crc = 0;
    for (uint16_t i = 0; i < len; i++) {
        crc ^= buf[i];
        for (uint8_t j = 0; j < 8; j++)
            crc = (crc & 0x80) ? ((crc << 1) ^ 0x07) : (crc << 1);
    }
    return crc;
}

/**
 * @brief Pack corrected data into a binary frame
 *        (only acceleration + angular rate, other fields filled with 0)
 *
 * @param pStr       Output buffer (>= 72 bytes)
 * @param pxSendData Pointer to DsImu_ImuCorrectedDataType returned by DsImu_GetData()
 * @return           Total frame length = 72
 *
 * Corresponds to the full format of protocol message 10h, but the
 * attitude/quaternion/temperature/status fields are zeroed.
 * The host can distinguish by frame length: 72 bytes = six axes only,
 * or by checking if the attitude fields are all zero.
 */
int Composer_PrintImu_Basic(unsigned char* pStr,
    DsImu_ImuCorrectedDataType* pxSendData)
{
    static uint8_t  u8cnt = 0;
    uint8_t         idx = 0;

    /* Frame header 3B */     memcpy(pStr + idx, (uint8_t[3]) { 0xA5, 0x5A, 0x00 }, 3); idx += 3;
    /* Command word 2B */     memcpy(pStr + idx, (uint8_t[2]) { 0x10, 0x40 }, 2);      idx += 2;
    /* Frame counter 1B */    pStr[idx] = u8cnt = (u8cnt + 1) % 256;               idx += 1;
    /* Timestamp 8B */
    uint64_t t = (uint64_t)k_uptime_get();
    memcpy(pStr + idx, &t, 8);
    idx += 8;

    /* ★ Acceleration 12B (fAlignedAcceleration[3], m/s²) ★ */
    memcpy(pStr + idx, pxSendData->fAlignedAcceleration, 12);              idx += 12;

    /* ★ Angular rate 12B (fAlignedAngularRate[3], °/s) ★ */
    memcpy(pStr + idx, pxSendData->fAlignedAngularRate, 12);               idx += 12;

    /* ---- The following fields are not output in this example, filled with 0 (31B) ---- */
    /* Pitch(4) + Roll(4) + Yaw(4) + Quat(16) + Temp(2) + Status(1) = 31B */
    uint8_t zero31[31] = { 0 };
    memcpy(pStr + idx, zero31, 31);                                         idx += 31;

    /* CRC-8 1B */          pStr[idx] = CRC_8(pStr, idx);                 idx += 1;
    /* Frame tail 2B */     memcpy(pStr + idx, (uint8_t[2]) { 0x0D, 0x0A }, 2); idx += 2;

    return idx; /* = 72 */
}


/* ========================================================================
 *  4. Main Program
 * ======================================================================== */

#define BUF_SIZE  128
static uint8_t                    g_buf[BUF_SIZE];
static DsImu_ImuCorrectedDataType g_cor;   /* Corrected data */
static DsImu_ImuRawDataType       g_raw;   /* Raw data (for debugging, can pass NULL) */

#define STACK_SIZE 2048
K_THREAD_STACK_DEFINE(imu_stack, STACK_SIZE);

/**
 * @brief IMU data acquisition thread
 *
 * Workflow:
 *   DsImu_Init() → Loop DsImu_GetData() → Composer_PrintImu_Basic() → Serial send
 */
void imu_thread(void* p1, void* p2, void* p3)
{
    /* ---- Step A: Initialize SDK ---- */
    printk("[basic] Initializing DsImuSdk...\n");
    DsImuInitStatus_t ret = DsImu_Init(&CfgInfo);
    if (ret != INIT_SUCCESS) {
        printk("[basic] ERROR: DsImu_Init failed (code=%d)\n", ret);
        return;
    }
    printk("[basic] Init OK, starting data loop...\n");

    /* ---- Step B: Main loop ---- */
    while (1) {
        /*
         * ★ Core API: DsImu_GetData()
         *
         * Parameters:
         *   pxImuCorData — [out] Corrected data (acceleration, angular rate, attitude, quaternion, etc.)
         *   pxImuRawData — [out] Raw ADC data (can pass NULL if not needed)
         *
         * Return values:
         *   GET_SUCCESS         — New data available
         *   GET_DATA_NOT_READY  — Data not ready (normal, wait for next)
         *   GET_ERR_RESET       — IMU abnormal reset, need to call DsImu_Init() again
         *   Other               — Communication errors, etc.
         */
        DsImuGetDataStatus_t st = DsImu_GetData(&g_cor, &g_raw);

        switch (st) {
            case GET_SUCCESS: {
                /* Data ready → pack and send */
                int len = Composer_PrintImu_Basic(g_buf, &g_cor);
                /* TODO: uart_send(g_buf, len); */
                (void)len;

                /* Debug print example: */
                /*
                printk("Acc: %+.2f %+.2f %+.2f m/s² | Gyr: %+.2f %+.2f %+.2f °/s\n",
                       g_cor.fAlignedAcceleration[0],
                       g_cor.fAlignedAcceleration[1],
                       g_cor.fAlignedAcceleration[2],
                       g_cor.fAlignedAngularRate[0],
                       g_cor.fAlignedAngularRate[1],
                       g_cor.fAlignedAngularRate[2]);
                */
                break;
            }
            case GET_ERR_RESET:
                /* IMU abnormal reset → re-initialize */
                printk("[basic] IMU reset! Re-init...\n");
                DsImu_Init(&CfgInfo);
                break;
            case GET_DATA_NOT_READY:
                /* No new data, normal */
                break;
            default:
                printk("[basic] GetData error: %d\n", st);
                break;
        }

        k_usleep(2000);  /* 500Hz = 2ms interval */
    }
}

/* ---- Entry ---- */
int main(void)
{
    printk("=== Basic 6-Axis Output Example ===\n");

    g_spi_dev = DEVICE_DT_GET(DT_NODELABEL(spi2));
    g_gpio_dev = DEVICE_DT_GET(DT_NODELABEL(gpiob));
    if (!device_is_ready(g_spi_dev) || !device_is_ready(g_gpio_dev))
        return -1;

    gpio_pin_configure(g_gpio_dev, 9, GPIO_OUTPUT_HIGH);
    gpio_pin_configure(g_gpio_dev, 8, GPIO_OUTPUT_HIGH);
    gpio_pin_configure(g_gpio_dev, 4, GPIO_OUTPUT_HIGH);
    TickInit();

    k_thread_create((struct k_thread*)k_malloc(sizeof(struct k_thread)),
        imu_stack, K_THREAD_STACK_SIZEOF(imu_stack),
        imu_thread, NULL, NULL, NULL, 6, 0, K_NO_WAIT);

    while (1) k_sleep(K_FOREVER);
    return 0;
}