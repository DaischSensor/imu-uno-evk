/**
 * @file    attitude_output_example.c
 * @brief   Attitude / Quaternion output example
 *
 * [Function]
 *   Demonstrates how to use DsImuSdk to obtain full IMU data (acceleration +
 *   angular rate + attitude + quaternion + temperature + AHRS status),
 *   outputting via the complete binary protocol.
 *
 *
 * [SDK API Call Flow]
 *
 *   ┌──────────────────────────────────────────────────────┐
 *   │ 1. Fill DsImu_CfgInfoType (range/rate/HAL pointers)  │
 *   │ 2. DsImu_Init(&CfgInfo)          → initialize        │
 *   │ 3. DsImu_GetData(&cor, &raw)   → Read data in a loop │
 *   │ 4. Composer_PrintImu()           → full packet out   │
 *   └──────────────────────────────────────────────────────┘
 *
 * [Coordinate System Definition]
 *
 *   Body Frame (right‑hand rule):
 *     X — forward, Y — right, Z — down
 *
 *   Euler angles (Z‑Y‑X rotation, unit: °):
 *     Pitch — about Y axis, nose‑up positive, range ±90°
 *     Roll  — about X axis, right‑wing‑down positive, range ±180°
 *     Yaw   — about Z axis, north‑by‑east positive, range 0~360°
 *
 *   Quaternion (q = w + xi + yj + zk):
 *     Rotation from body frame to navigation frame.
 *     q0=w (scalar), q1=x, q2=y, q3=z
 *
 *
 * [Complete Binary Frame (72 bytes) — 0x10 Application Packet]
 *
 *   D0-D2:   Header 0xA5 0x5A 0x00           (3B)
 *   D3-D4:   Command 0x10 0x40               (2B)
 *   D5:      Frame count 0~255               (1B)
 *   D6-D13:  Timestamp UInt64 ms LE          (8B)
 *   D14-D25: Acceleration X/Y/Z Float×3 m/s² (12B)
 *   D26-D37: Angular rate X/Y/Z Float×3 °/s  (12B)
 *   D38-D41: Pitch Float °                   (4B)
 *   D42-D45: Roll Float °                    (4B)
 *   D46-D49: Yaw Float °                     (4B)
 *   D50-D65: Quaternion q0~q3 Float×4        (16B)
 *   D66-D67: Temperature Int16 ×0.1 ℃ LE    (2B)
 *   D68:     IMU Status UInt8                (1B)
 *   D69:     CRC-8                           (1B)
 *   D70-D71: Footer 0x0D 0x0A                (2B)
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/drivers/gpio.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

#include "DsImuSdk.h"


 /* ========================================================================
  *  1. HAL Layer (same as basic_output_example, abbreviated here)
  * ======================================================================== */
static const struct device* g_spi_dev;
static const struct device* g_gpio_dev;
static struct spi_config g_spi_cfg = {
    .frequency = 1000000,
    .operation = SPI_OP_MODE_MASTER | SPI_TRANSFER_MSB | SPI_WORD_SET(8),
    .slave = 0, .cs = NULL,
};

static void cs_select(DsImuChipId_t id) { /* ... see basic_output_example */ }
static void cs_deselect(DsImuChipId_t id) { /* ... see basic_output_example */ }

uint8_t spiTransfer(DsImuChipId_t id, DsImu_SpiBuffType* buf, uint8_t n)
{ /* ... see basic_output_example */ return 0;
}
void spiCancel(DsImuChipId_t id) { cs_deselect(id); }
uint8_t spiGetTransferResult(DsImuChipId_t id) { (void)id; return 0; }

static uint64_t g_cps;
void TickInit(void) { g_cps = sys_clock_hw_cycles_per_sec(); }
uint64_t GetTickUs(void) {
    uint64_t c = k_cycle_get_64();
    return (c / g_cps) * 1000000ULL + (c % g_cps) * 1000000ULL / g_cps;
}


/* ========================================================================
 *  2. SDK Configuration
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
 *  3. Complete Binary Protocol Packet Assembly (72 bytes, including all fields)
 * ======================================================================== */

static uint8_t CRC_8(const uint8_t* buf, uint16_t len)
{
    uint8_t c = 0;
    for (uint16_t i = 0; i < len; i++) {
        c ^= buf[i];
        for (uint8_t j = 0; j < 8; j++)
            c = (c & 0x80) ? ((c << 1) ^ 0x07) : (c << 1);
    }
    return c;
}

/**
 * @brief Full packet assembly — outputs all fields
 *
 * @note  Attitude output order: Pitch → Roll → Yaw (consistent with protocol spec).
 *        DsImu_ImuCorrectedDataType.fAttitude[] = {Yaw, Pitch, Roll},
 *        so reordering is needed.
 */
int Composer_PrintImu(unsigned char* pStr,
    DsImu_ImuCorrectedDataType* pxSendData)
{
    static uint8_t  u8cnt = 0;
    uint8_t         idx = 0;

    /* Header 3B */     memcpy(pStr + idx, (uint8_t[3]) { 0xA5, 0x5A, 0x00 }, 3); idx += 3;
    /* Command 2B */    memcpy(pStr + idx, (uint8_t[2]) { 0x10, 0x40 }, 2);      idx += 2;
    /* Frame count 1B */pStr[idx] = u8cnt = (u8cnt + 1) % 256;               idx += 1;
    /* Timestamp 8B */ {
        uint64_t t = (uint64_t)k_uptime_get();
        memcpy(pStr + idx, &t, 8);
    }                       idx += 8;

    /* Acceleration X/Y/Z 12B */
    memcpy(pStr + idx, pxSendData->fAlignedAcceleration, 12);              idx += 12;

    /* Angular rate X/Y/Z 12B */
    memcpy(pStr + idx, pxSendData->fAlignedAngularRate, 12);               idx += 12;

    /*
     * ★ Attitude 12B — note the order
     *   fAttitude[0]=Yaw, [1]=Pitch, [2]=Roll
     *   Protocol order: Pitch → Roll → Yaw
     */
    memcpy(pStr + idx, &pxSendData->fAttitude[1], 4);  idx += 4;  /* Pitch */
    memcpy(pStr + idx, &pxSendData->fAttitude[2], 4);  idx += 4;  /* Roll  */
    memcpy(pStr + idx, &pxSendData->fAttitude[0], 4);  idx += 4;  /* Yaw   */

    /* ★ Quaternion q0~q3 16B  */
    memcpy(pStr + idx, pxSendData->fQuat, 16);                            idx += 16;

    /* ★ Temperature 2B (Int16, scale 0.1 ℃)  */
    {
        int16_t t = (int16_t)(10.0F * pxSendData->fTemperature);
        memcpy(pStr + idx, &t, 2);
    }                                                                       idx += 2;

    /* ★ IMU Status 1B ★ */
    pStr[idx] = (uint8_t)(pxSendData->u16AhrsStatus & 0xFFU);              idx += 1;

    /* CRC-8 1B */    pStr[idx] = CRC_8(pStr, idx);                        idx += 1;
    /* Footer 2B */   memcpy(pStr + idx, (uint8_t[2]) { 0x0D, 0x0A }, 2);  idx += 2;

    return idx; /* = 72 */
}


/* ========================================================================
 *  4. Main Program — with AHRS Convergence Wait
 * ======================================================================== */

#define BUF_SIZE  128
#define AHRS_TIMEOUT_MS  5000

static uint8_t                    g_buf[BUF_SIZE];
static DsImu_ImuCorrectedDataType g_cor;
static DsImu_ImuRawDataType       g_raw;

#define STACK_SIZE 3072
K_THREAD_STACK_DEFINE(imu_stack, STACK_SIZE);

/**
 * @brief IMU thread with attitude output
 *
 * Key steps:
 *   1. DsImu_Init() initialization
 *   2. (Optional) DsImu_GetInfo() read firmware version/serial number
 *   3. Loop DsImu_GetData() until AHRS converges
 *   4. Enter normal data output loop
 */
void imu_thread(void* p1, void* p2, void* p3)
{
    /* ---- Step 1: Initialization ---- */
    printk("[att] Initializing DsImuSdk...\n");
    DsImuInitStatus_t ret = DsImu_Init(&CfgInfo);
    if (ret != INIT_SUCCESS) {
        printk("[att] ERROR: Init failed (code=%d)\n", ret);
        return;
    }
    printk("[att] SDK Init OK.\n");

    /* ---- Step 2: (Optional) Get device info ---- */
    {
        DsImu_InfoType info;
        if (DsImu_GetInfo(&info) == GET_INFO_SUCCESS) {
            printk("[att] SN: %02X%02X%02X%02X%02X%02X%02X%02X, "
                "FW: %s\n",
                info.sn[0], info.sn[1], info.sn[2], info.sn[3],
                info.sn[4], info.sn[5], info.sn[6], info.sn[7],
                info.version);
        }
    }

    /* ---- Step 3: Wait for AHRS convergence ---- */
    printk("[att] Waiting for AHRS convergence (max %dms)...\n",
        AHRS_TIMEOUT_MS);

    bool ahrs_ok = false;
    for (uint32_t t = 0; t < AHRS_TIMEOUT_MS; t += 100) {
        if (DsImu_GetData(&g_cor, &g_raw) == GET_SUCCESS) {
            uint8_t st = (uint8_t)(g_cor.u16AhrsStatus & 0xFFU);
            bool imu_ok = (st >> 6) & 1;  /* Bit 6: valid IMU      */
            bool inited = (st >> 4) & 1;  /* Bit 4: initialized     */
            bool att_ok = (st >> 5) & 1;  /* Bit 5: valid attitude  */

            if (imu_ok && inited && att_ok) {
                ahrs_ok = true;
                printk("[att] AHRS converged after %ums\n", t);
                break;
            }
        }
        k_msleep(100);
    }
    if (!ahrs_ok) {
        printk("[att] WARNING: AHRS not fully converged. "
            "Attitude may be unreliable.\n");
    }

    /* ---- Step 4: Main data loop ---- */
    printk("[att] Starting data output...\n");

    while (1) {
        DsImuGetDataStatus_t st = DsImu_GetData(&g_cor, &g_raw);

        switch (st) {
            case GET_SUCCESS: {
                /* Check attitude validity */
                uint8_t flags = (uint8_t)(g_cor.u16AhrsStatus & 0xFFU);
                bool att_valid = (flags >> 5) & 1;
                bool is_static = (flags >> 3) & 1;

                /* Packet assembly (full 72 bytes) */
                int len = Composer_PrintImu(g_buf, &g_cor);
                /* TODO: uart_send(g_buf, len); */
                (void)len;

                /* Debug print: */
                if (att_valid) {
                    /*
                    printk("P:%+6.1f° R:%+6.1f° Y:%+6.1f° | "
                           "Q:[%.3f %.3f %.3f %.3f] | "
                           "T:%.1f°C | %s\n",
                           g_cor.fAttitude[1],
                           g_cor.fAttitude[2],
                           g_cor.fAttitude[0],
                           g_cor.fQuat[0], g_cor.fQuat[1],
                           g_cor.fQuat[2], g_cor.fQuat[3],
                           g_cor.fTemperature,
                           is_static ? "STATIC" : "MOVING");
                    */
                }
                break;
            }
            case GET_ERR_RESET:
                printk("[att] IMU reset → re-init\n");
                DsImu_Init(&CfgInfo);
                break;
            case GET_DATA_NOT_READY:
                break;
            default:
                printk("[att] GetData error: %d\n", st);
                break;
        }

        k_usleep(2000);
    }
}

/* ---- Entry point ---- */
int main(void)
{
    printk("=== Attitude + Quaternion Output Example ===\n");

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