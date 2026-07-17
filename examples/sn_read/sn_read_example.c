/**
 * @file    sn_read_example.c
 * @brief   SN code / firmware version read example
 *
 * 【Function】
 *   Demonstrates how to use DsImuSdk to read IMU module information:
 *     - SN code
 *     - Firmware version
 *
 *   Output: printk()
 *
 * 【SDK API call flow】
 *
 *   DsImu_Init(&CfgInfo)     → Initialize SDK
 *   DsImu_GetInfo(&info)     → Read device information
 *   printk(...)              → Print SN and version
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/drivers/gpio.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>

#include "DsImuSdk.h"


 /* ========================================================================
  *  1. HAL layer
  * ======================================================================== */
static const struct device* g_spi_dev;
static const struct device* g_gpio_dev;
static struct spi_config g_spi_cfg = {
    .frequency = 1000000,
    .operation = SPI_OP_MODE_MASTER | SPI_TRANSFER_MSB | SPI_WORD_SET(8),
    .slave = 0, .cs = NULL,
};

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

void spiCancel(DsImuChipId_t eChipId) { cs_deselect(eChipId); }
uint8_t spiGetTransferResult(DsImuChipId_t eChipId) { (void)eChipId; return 0; }

static uint64_t g_cps;
void TickInit(void) { g_cps = sys_clock_hw_cycles_per_sec(); }
uint64_t GetTickUs(void)
{
    uint64_t c = k_cycle_get_64();
    return (c / g_cps) * 1000000ULL + (c % g_cps) * 1000000ULL / g_cps;
}


/* ========================================================================
 *  2. SDK configuration
 * ======================================================================== */
DsImu_CfgInfoType CfgInfo = {
#if defined(IMU_TARGET_IM3J)
    .module_type = MODULE_IM3J,
    .acc_fs = IM3J_FS_XL_6_G,
    .gyr_fs = IM3J_FS_GY_1000_DPS,
    .acc_odr = IM3J_XL_ODR_200_HZ,
    .acc_lpf = IM3J_XL_LPF_OSR4,
    .gyr_lpf = IM3J_GY_ODR_200_LPF_23_HZ,
#elif defined(IMU_TARGET_IM8S)
    .module_type = MODULE_IM8S,
    .acc_fs = IM8S_FS_XL_8_G,
    .gyr_fs = IM8S_FS_GY_250_DPS,
    .acc_odr = BYPASS,
    .acc_lpf = IM8S_XL_LPF_16HZ,
    .gyr_lpf = IM8S_GY_LPF_50HZ,
#endif
    .tim_freq = FREQ_500HZ,
    .u64UsTickValueMax = 0xFFFFFFFFFFFFFFFFU,
    .mcu_freq = 160U,
    .u32SpiTimeoutUs = {10000U, 10000U},

    .transfer = spiTransfer,
    .cancel = spiCancel,
    .systick = GetTickUs,
    .getStatus = spiGetTransferResult,

    .fMntAng = {0.0F, 0.0F, 0.0F},
    .fExtAng = {0.0F, 0.0F, 0.0F},
    .fPos = {0.0F, 0.0F, 0.0F},
};


/* ========================================================================
 *  3. Main program
 * ======================================================================== */
int main(void)
{
    printk("\n");
    printk("================================================\n");
    printk("  DS-IMU  SN / Firmware Version Read Example\n");
    printk("================================================\n");
    printk("\n");

    /* ---- Hardware initialization ---- */
    g_spi_dev = DEVICE_DT_GET(DT_NODELABEL(spi2));
    g_gpio_dev = DEVICE_DT_GET(DT_NODELABEL(gpiob));

    if (!device_is_ready(g_spi_dev)) {
        printk("[ERROR] SPI device not ready!\n");
        return -1;
    }
    if (!device_is_ready(g_gpio_dev)) {
        printk("[ERROR] GPIO device not ready!\n");
        return -1;
    }

    gpio_pin_configure(g_gpio_dev, 9, GPIO_OUTPUT_HIGH);
    gpio_pin_configure(g_gpio_dev, 8, GPIO_OUTPUT_HIGH);
    gpio_pin_configure(g_gpio_dev, 4, GPIO_OUTPUT_HIGH);
    TickInit();

    printk("[HW] SPI & GPIO OK\n");

    /* ---- Step 1: Initialize IMU SDK ---- */
    printk("[SDK] Initializing...\n");

    DsImuInitStatus_t init_ret = DsImu_Init(&CfgInfo);
    if (init_ret != INIT_SUCCESS) {
        printk("[ERROR] DsImu_Init failed! code = %d\n", init_ret);
        return -1;
    }

    printk("[SDK] Init OK\n");

    /* ---- Step 2: Read device information ---- */
    DsImu_InfoType xInfoGetSN;
    memset(&xInfoGetSN, 0, sizeof(xInfoGetSN));

    uint8_t ret = DsImu_GetInfo(&xInfoGetSN);

    if (ret == GET_INFO_SUCCESS) {
        uint16_t sn_last = ((uint16_t)xInfoGetSN.sn[3] << 8) | xInfoGetSN.sn[4];

        printk("\n");
        printk("================================================\n");
        printk("  Device Information\n");
        printk("================================================\n");
        printk("  SN:      %02u%02u%02u%04u\n",
            xInfoGetSN.sn[0],
            xInfoGetSN.sn[1],
            xInfoGetSN.sn[2],
            sn_last);
        printk("  Version: %s\n", xInfoGetSN.version);
        printk("================================================\n");
        printk("\n");

    }
    else {
        printk("[ERROR] DsImu_GetInfo failed! code = %d\n", ret);
        printk("        GET_INFO_SUCCESS       = 0\n");
        printk("        GET_INFO_ERR_UNINIT    = 1 (SDK not initialized)\n");
        printk("        GET_INFO_NULL_PARA     = 2 (parameter is NULL)\n");
        printk("        GET_INFO_SN_CRC_ERROR  = 3 (SN CRC mismatch)\n");
    }

    printk("Done. SN read example finished.\n");

    return 0;
}