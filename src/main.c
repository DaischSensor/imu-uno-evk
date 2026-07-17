#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/drivers/counter.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>
#include <string.h>
#include <stdbool.h>

#include "DsImuSdk.h"
#include "zephyr_spi_hal.h"
#include "zephyr_async_uart.h"
#include "zephyr_systick.h"
#include "uart_protocol.h"
#include "nvs_config.h"

#define DsImuGetData_STACK_SIZE 3072
#define DsImuGetData_PRIORITY 6
#define UART1Send_STACK_SIZE 3072
#define UART1Send_PRIORITY 5
#define IMU_SAMPLE_PERIOD_US   (2000U)
#define IMU_FIFO_DATA_ARRAY_SIZE (10U)

K_THREAD_STACK_DEFINE(DsImuGetData_stack_area, DsImuGetData_STACK_SIZE);
K_THREAD_STACK_DEFINE(UART1Send_stack_area, UART1Send_STACK_SIZE);
K_SEM_DEFINE(uart_output, 0, 10);
K_SEM_DEFINE(imu_sample, 0, 10);

static const struct device* g_pCounter_dev = DEVICE_DT_GET(DT_CHILD(DT_NODELABEL(timers2), counter));

#define TIMER_NODE DT_ALIAS(my_counter)
static struct counter_alarm_cfg g_xImuSampleAlarm_cfg;
static struct counter_alarm_cfg g_xUartOutputAlarm_cfg;

static uint8_t SerialSendBuffer[SERIAL_SEND_BUFFER_SIZE];
static uint8_t UcSerialCmdBuffer[SERIAL_CMD_BUFF_SIZE];

bool gImuOutputEnabled = false;
bool gImuReinitFlag = false;
bool gImuSerialOutputEnabled = true;

static uint16_t SerialBufferIdx = 0;
static uint32_t sRxLen = 0;
//static uint32_t sUartProcDelayCompUs = 100;

static uint32_t gUartCurrentBaud = 921600U;   // Actual hardware baud rate
static uint32_t gUartDataBaud = 921600U;      // Configured baud rate for data output

static uint32_t gUartSendPeriodUs = 10000U;
/* Compensation control variables */
static uint64_t g_u64UartOutputExpectedUs = 0U;
static uint64_t g_u64ImuSampleExpectedUs = 0U;

typedef struct {
    DsImu_ImuCorrectedDataType xImuCorDataArray[IMU_FIFO_DATA_ARRAY_SIZE];
    uint8_t u8LatestIndex;
} Imu_Fifo_t;

static Imu_Fifo_t g_xImuFifo;

struct k_thread DsImuGetData_thread;
struct k_thread UART1Send_thread;

__attribute__((constructor(101))) static void __early_disable_alignment_check(void)
{
    *(volatile uint32_t*)0xE000ED14 &= ~(1UL << 3);
    __asm__ volatile("dsb sy" : : : "memory");
    __asm__ volatile("isb sy" : : : "memory");
}

void EnsureAlignmentCheckDisabled(void)
{
    static uint8_t checked = 0;
    if (!checked)
    {
        *(volatile uint32_t*)0xE000ED14 &= ~(1UL << 3);
        __asm__ volatile("dsb sy");
        __asm__ volatile("isb sy");
        checked = 1;
    }
}

static void Imu_fifo_push(const DsImu_ImuCorrectedDataType xImuCorData)
{
    uint8_t u8LatestIndex = g_xImuFifo.u8LatestIndex;
    u8LatestIndex = (u8LatestIndex + 1U) % IMU_FIFO_DATA_ARRAY_SIZE;
    (void)memcpy(&g_xImuFifo.xImuCorDataArray[u8LatestIndex], &xImuCorData, sizeof(DsImu_ImuCorrectedDataType));
    g_xImuFifo.u8LatestIndex = u8LatestIndex;
    if (g_xImuFifo.u8LatestIndex > IMU_FIFO_DATA_ARRAY_SIZE - 1U) {
        g_xImuFifo.u8LatestIndex = IMU_FIFO_DATA_ARRAY_SIZE - 1U;
    }
}

static void timer_callback(const struct device* dev, uint8_t chan_id, uint32_t ticks, void* user_data)
{
    (void)ticks;
    (void)user_data;
    uint64_t u64NowUs = k_cyc_to_us_floor64(k_cycle_get_64());
    int64_t i64Drift = 0;
    uint64_t u64AlarmUs = 0U;
    if (0U == chan_id) {
        /* Calculate expected trigger time */
        if (0U == g_u64ImuSampleExpectedUs) {
            /* First trigger, initialize */
            g_u64ImuSampleExpectedUs = u64NowUs;
        }
        else {
            g_u64ImuSampleExpectedUs += IMU_SAMPLE_PERIOD_US;
        }
        /* Calculate drift (positive = late, negative = early) */
        i64Drift = (int64_t)(u64NowUs - g_u64ImuSampleExpectedUs);

        /* ========== Core compensation algorithm ========== */
        /* If drift positive (late), advance next; if negative (early), delay next */
        u64AlarmUs = (uint64_t)((int64_t)IMU_SAMPLE_PERIOD_US - i64Drift);
        if ((u64AlarmUs < (IMU_SAMPLE_PERIOD_US - 100U)) || (u64AlarmUs > (IMU_SAMPLE_PERIOD_US + 100U))) {
            u64AlarmUs = IMU_SAMPLE_PERIOD_US;
        }
        g_xImuSampleAlarm_cfg.ticks = counter_us_to_ticks(dev, u64AlarmUs);
        //printk("now_us:%llu,expected_us:%llu,drift:%lld,alarm_us:%llu us\n", u64NowUs, g_u64ImuSampleExpectedUs, i64Drift, u64AlarmUs);
        if (true == gImuOutputEnabled) {
            counter_set_channel_alarm(dev, chan_id, &g_xImuSampleAlarm_cfg);
        }

        k_sem_give(&imu_sample);
    }
    else if (1U == chan_id) {
        /* Calculate expected trigger time */
        if (0U == g_u64UartOutputExpectedUs) {
            /* First trigger, initialize */
            g_u64UartOutputExpectedUs = u64NowUs;
        }
        else {
            g_u64UartOutputExpectedUs += gUartSendPeriodUs;
        }
        /* Calculate drift (positive = late, negative = early) */
        i64Drift = (int64_t)(u64NowUs - g_u64UartOutputExpectedUs);

        /* ========== Core compensation algorithm ========== */
        /* If drift positive (late), advance next; if negative (early), delay next */
        u64AlarmUs = (uint64_t)((int64_t)gUartSendPeriodUs - i64Drift);
        if ((u64AlarmUs < (gUartSendPeriodUs - 100U)) || (u64AlarmUs > (gUartSendPeriodUs + 100U))) {
            u64AlarmUs = gUartSendPeriodUs;
        }

        g_xUartOutputAlarm_cfg.ticks = counter_us_to_ticks(dev, u64AlarmUs);
        //printk("now_us:%llu,expected_us:%llu,drift:%lld,alarm_us:%llu us\n", u64NowUs, g_u64UartOutputExpectedUs, i64Drift, u64AlarmUs);

        counter_set_channel_alarm(dev, chan_id, &g_xUartOutputAlarm_cfg);
        k_sem_give(&uart_output);
    }
    else { /* do nothing. */
    }
}

static void UartCmdRxCallback(uint8_t* data, size_t len);

#if defined(IMU_TARGET_IM3J)
DsImu_CfgInfoType CfgInfo = {
    .module_type = MODULE_IM3J,
    .acc_fs = IM3J_FS_XL_6_G, /**< Accelerometer full-scale code. */
    .gyr_fs = IM3J_FS_GY_1000_DPS, /**< Gyroscope full-scale code. */
    .acc_odr = IM3J_XL_ODR_200_HZ, /**< Accelerometer output data rate. */
    .acc_lpf = IM3J_XL_LPF_OSR4, /**< Accelerometer low-pass filter. */
    .gyr_lpf = IM3J_GY_ODR_200_LPF_23_HZ, /**< Gyroscope ODR and LPF. */
    .tim_freq = FREQ_500HZ, /**< Timer frequency [Hz]. */
    .u32SpiTimeoutUs[0] = 10000U, /**< [us] */
    .u32SpiTimeoutUs[1] = 10000U, /**< [us] */
    .u64UsTickValueMax = 0xFFFFFFFFFFFFFFFFU,
    .mcu_freq = 160U,
    .transfer = spiTransfer,
    .cancel = spiCancel,
    .systick = GetTickUs,
    .getStatus = spiGetTransferResult,
    .fMntAng = {0.0F, 0.0F, 0.0F},
    .fExtAng = {0.0F, 0.0F, 0.0F},
    .fPos = {0.0F, 0.0F, 0.0F}
};

#elif defined(IMU_TARGET_IM8S)
DsImu_CfgInfoType CfgInfo = {
    .module_type = MODULE_IM8S,
    .acc_fs = IM8S_FS_XL_8_G, /**< Accelerometer full-scale code. */
    .gyr_fs = IM8S_FS_GY_250_DPS, /**< Gyroscope full-scale code. */
    .acc_odr = BYPASS, /**< Accelerometer ODR. */
    .acc_lpf = IM8S_XL_LPF_16HZ, /**< Accelerometer low-pass filter. */
    .gyr_lpf = IM8S_GY_LPF_50HZ, /**< Gyroscope low-pass filter. */
    .tim_freq = FREQ_500HZ, /**< Timer frequency [Hz]. */
    .u32SpiTimeoutUs[0] = 10000U, /**< [us] */
    .u32SpiTimeoutUs[1] = 10000U, /**< [us] */
    .u64UsTickValueMax = 0xFFFFFFFFFFFFFFFFU,
    .mcu_freq = 160U,
    .transfer = spiTransfer,
    .cancel = spiCancel,
    .systick = GetTickUs,
    .getStatus = spiGetTransferResult,
    .fMntAng = {0.0F, 0.0F, 0.0F},
    .fExtAng = {0.0F, 0.0F, 0.0F},
    .fPos = {0.0F, 0.0F, 0.0F}
};
#endif

static DsImu_ImuCorrectedDataType xImuCorData;
static DsImu_ImuRawDataType xImuRawData;
static DsImu_InfoType xInfo;
static bool sDsImuSdkInitFlag = false;

static void Counter_Config(const struct device* counter_dev)
{
    int ret = 0;
    uint32_t u32ImuSamplePeriodUs = IMU_SAMPLE_PERIOD_US;  /* 2ms */
    uint32_t u32ImuSampleAlarmTicks = 0U;
    uint32_t u32UartOutputPeriodUs = gUartSendPeriodUs;
    uint32_t u32UartOutputAlarmTicks = 0U;

    if (!device_is_ready(counter_dev)) {
        printk("Counter device not ready!\n");
        return;
    }
    printk("Counter device found: %s\n", counter_dev->name);

    /* Get timer actual frequency (for validation) */
    uint32_t freq = counter_get_frequency(counter_dev);
    printk("Timer frequency: %u Hz\n", freq);

    /* Convert Imu microseconds to ticks (based on actual frequency) */
    u32ImuSampleAlarmTicks = counter_us_to_ticks(counter_dev, u32ImuSamplePeriodUs);
    printk("ImuSamplePeriodUs: %u us -> %u ticks\n", u32ImuSamplePeriodUs, u32ImuSampleAlarmTicks);
    g_xImuSampleAlarm_cfg.callback = timer_callback;
    g_xImuSampleAlarm_cfg.ticks = u32ImuSampleAlarmTicks;
    g_xImuSampleAlarm_cfg.user_data = NULL;
    g_xImuSampleAlarm_cfg.flags = 0U;

    //   ret = counter_set_channel_alarm(counter_dev, 0, &g_xImuSampleAlarm_cfg);
    //   if (ret != 0) {
    //       printk("Failed to set alarm (err: %d)\n", ret);
    //       return;
    //   }

    u32UartOutputAlarmTicks = counter_us_to_ticks(counter_dev, u32UartOutputPeriodUs);
    printk("UartOutputPeriodUs: %u us -> %u ticks\n", u32UartOutputPeriodUs, u32UartOutputAlarmTicks);
    g_xUartOutputAlarm_cfg.callback = timer_callback;
    g_xUartOutputAlarm_cfg.ticks = u32UartOutputAlarmTicks;
    g_xUartOutputAlarm_cfg.user_data = NULL;
    g_xUartOutputAlarm_cfg.flags = 0U;

    ret = counter_set_channel_alarm(counter_dev, 1, &g_xUartOutputAlarm_cfg);
    if (ret != 0) {
        printk("Failed to set alarm (err: %d)\n", ret);
        return;
    }

    ret = counter_start(counter_dev);
    if (ret != 0) {
        printk("Failed to start counter (err: %d)\n", ret);
        return;
    }

    printk("Timer started!\n");
}

// static uint64_t g_u64LastTick = 0U;
void DsImuGetData_entry_point(void* p1, void* p2, void* p3)
{
#if defined(IMU_TARGET_IM3J)
    for (;;)
    {
        if ((gImuOutputEnabled == false) || (gImuReinitFlag))
        {
            if (gImuReinitFlag)
            {
                gImuReinitFlag = 0;
                //printk("Reinit IMU...\n");
            }
            else
            {
                //printk("IMU First Start...\n");
            }

            if (DsImu_Init(&CfgInfo) == 0)
            {
                //printk("DsImu SDK Init Success!\n");
                gImuOutputEnabled = true;
                g_u64ImuSampleExpectedUs = 0U;
                counter_set_channel_alarm(g_pCounter_dev, 0, &g_xImuSampleAlarm_cfg);
            }
            else
            {
                //printk("IMU Init FAIL\n");
            }
        }
        k_sem_take(&imu_sample, K_FOREVER);
        //uint64_t u64Us = k_cyc_to_us_floor64(k_cycle_get_64());
        //uint64_t u64diff = u64Us - g_u64LastTick;
        //g_u64LastTick = u64Us;
        //uint64_t u64Us2 = 0U;
        //uint64_t u64Us3 = 0U;
        if (gImuOutputEnabled)
        {
            //u64Us2 = k_cyc_to_us_floor64(k_cycle_get_64());
            DsImuGetDataStatus_t eGetDataStatus =
                DsImu_GetData(&xImuCorData, &xImuRawData);
            //u64Us3 = k_cyc_to_us_floor64(k_cycle_get_64());
            Imu_fifo_push(xImuCorData);
            if (eGetDataStatus == GET_SUCCESS)
            {

            }
            else if (eGetDataStatus == GET_DATA_NOT_READY)
            {

            }
            else if (eGetDataStatus == GET_ERR_RESET)
            {
                gImuOutputEnabled = false;
                gImuReinitFlag = true;
            }
            else
            {
                //printk("DsImu Get Data Error! Code: %d\n", eGetDataStatus);
            }
        }
        //printk("Tick:%llu, diff:%llu us, GetDataRun:%llu us\n", u64Us, u64diff, u64Us3 - u64Us2);
//k_msleep(10);
    }

#elif defined(IMU_TARGET_IM8S)

    for (;;)
    {
        if ((gImuOutputEnabled == false) || (gImuReinitFlag))
        {
            if (gImuReinitFlag)
            {
                gImuReinitFlag = 0;
                //printk("Reinit IMU...\n");
            }
            else
            {
                //printk("IMU First Start...\n");
            }

            DsImuInitStatus_t ret = DsImu_Init(&CfgInfo);

            if (ret == INIT_SUCCESS)
            {
                gImuOutputEnabled = true;
                //printk("DsImu SDK Init Success!\n");
                uint8_t eStatus_GetInfo = DsImu_GetInfo(&xInfo);
                if (eStatus_GetInfo == 0U)
                {
                    //printk("DsImu SDK GetInfo Success!\n");
                }
                else {
                    //printk("DsImu SDK GetInfo Failed! Error Code: %d\n", eStatus_GetInfo);
                }
                sDsImuSdkInitFlag = true;
                g_u64ImuSampleExpectedUs = 0U;
                counter_set_channel_alarm(g_pCounter_dev, 0, &g_xImuSampleAlarm_cfg);
            }
            else
            {
                //printk("IMU Init FAIL! Error Code:%d\n", ret);
            }
        }
        k_sem_take(&imu_sample, K_FOREVER);
        //uint64_t u64Us = k_cyc_to_us_floor64(k_cycle_get_64());
        //uint64_t u64diff = u64Us - g_u64LastTick;
        //g_u64LastTick = u64Us;
        //uint64_t u64Us2 = 0U;
        //uint64_t u64Us3 = 0U;
        if (gImuOutputEnabled)
        {
            //u64Us2 = k_cyc_to_us_floor64(k_cycle_get_64());
            DsImuGetDataStatus_t eGetDataStatus =
                DsImu_GetData(&xImuCorData, &xImuRawData);
            //u64Us3 = k_cyc_to_us_floor64(k_cycle_get_64());
            Imu_fifo_push(xImuCorData);
            if (eGetDataStatus == GET_SUCCESS)
            {

            }
            else if (eGetDataStatus == GET_ERR_RESET)
            {
                gImuOutputEnabled = false;
                gImuReinitFlag = true;
            }
            else
            {
                //printk("DsImu Get Data Error! Code: %d\n", eGetDataStatus);
            }
        }
        //printk("Tick:%llu, diff:%llu us, GetDataRun:%llu us\n", u64Us, u64diff, u64Us3 - u64Us2);
//k_msleep(10);
    }
#endif
}

void UART1Send_entry_point(void* p1, void* p2, void* p3)
{
    while (1)
    {
        /* Wait for semaphore (block until data available) */
        k_sem_take(&uart_output, K_FOREVER);
        //uint64_t u64Us = k_cyc_to_us_floor64(k_cycle_get_64());
        //uint64_t u64diff = u64Us - g_u64LastTick;
        //g_u64LastTick = u64Us;
        if (gImuSerialOutputEnabled)
        {
            SerialBufferIdx = Composer_PrintImu(
                SerialSendBuffer,
                &g_xImuFifo.xImuCorDataArray[g_xImuFifo.u8LatestIndex]);

            SerialSendMsg(
                SerialSendBuffer,
                SerialBufferIdx);
            //printk("Tick:%llu, diff: %llu us\n", u64Us, u64diff);
            //printk("Running!\r\n");
        }
        //k_usleep(gUartSendPeriodUs - sUartProcDelayCompUs);
    }
}

void setUartSendPeriodUs(const uint32_t u32UartSendPeriodUs)
{
    gUartSendPeriodUs = u32UartSendPeriodUs;
    g_u64UartOutputExpectedUs = 0U;
    //printk("UartSetPeriod:%u\n", gUartSendPeriodUs);
}

uint32_t GetUartSendPeriodUs(void) { return gUartSendPeriodUs; }

// Current hardware baud rate
void setUartCurrentBaud(const uint32_t baud) { gUartCurrentBaud = baud; }
uint32_t GetUartCurrentBaud(void) { return gUartCurrentBaud; }

// Configured baud rate for data output
void setUartBaud(const uint32_t baud) { gUartDataBaud = baud; }
uint32_t GetUartBaud(void) { return gUartDataBaud; }


int main(void)
{
    EnsureAlignmentCheckDisabled();
    UartRxCbRegister(UartCmdRxCallback);

    const struct device* spi_dev = DEVICE_DT_GET(DT_NODELABEL(spi2));
    if (!device_is_ready(spi_dev))
    {
        //printk("SPI2 not ready\n");
        return 0;
    }

    //counter_dev = DEVICE_DT_GET_ANY(st_stm32_counter);
    if (g_pCounter_dev) {
        printk("The selected counter device is: %s\n", g_pCounter_dev->name);
    }

    TickInit();
    Zephyr_SPI_GPIO_Init();

    /* ====== Load NVS configuration ====== */
    int nvs_rc = NvsConfigInit();
    if (nvs_rc == 0) {
        StoredConfig_t stored;
        if (NvsConfigLoad(&stored) == 0) {
            /* Override default values */
            gUartDataBaud = stored.baud_rate;
            gUartSendPeriodUs = stored.send_period_us;
            CfgInfo.acc_fs = stored.acc_fs;
            CfgInfo.gyr_fs = stored.gyr_fs;
            CfgInfo.acc_odr = stored.acc_odr;
            CfgInfo.acc_lpf = stored.acc_lpf;
            CfgInfo.gyr_lpf = stored.gyr_lpf;
            printk("NVS: config loaded, baud=%u, period=%u\n",
                gUartDataBaud, gUartSendPeriodUs);
        }
    }

    Counter_Config(g_pCounter_dev);

    (void)memset(&g_xImuFifo.xImuCorDataArray[0], 0, sizeof(DsImu_ImuCorrectedDataType));
    g_xImuFifo.u8LatestIndex = 9U;

    k_thread_create(&DsImuGetData_thread,
        DsImuGetData_stack_area,
        K_THREAD_STACK_SIZEOF(DsImuGetData_stack_area),
        DsImuGetData_entry_point,
        NULL, NULL, NULL,
        DsImuGetData_PRIORITY,
        0,
        K_NO_WAIT);

    k_thread_create(&UART1Send_thread,
        UART1Send_stack_area,
        K_THREAD_STACK_SIZEOF(UART1Send_stack_area),
        UART1Send_entry_point,
        NULL, NULL, NULL,
        UART1Send_PRIORITY,
        0,
        K_NO_WAIT);
    return 0;
}

static void UartCmdRxCallback(uint8_t* data, size_t len)
{
    for (int i = 0; i < len; i++)
    {
        uint8_t byte = data[i];

        if (sRxLen >= SERIAL_CMD_BUFF_SIZE)
        {
            sRxLen = 0;
        }

        UcSerialCmdBuffer[sRxLen++] = byte;

        if (byte == '\n')
        {
            ProcessSerialBuffer(UcSerialCmdBuffer, &sRxLen);
        }
    }

    k_free(data);
}
