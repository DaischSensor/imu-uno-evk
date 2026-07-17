#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/uart.h>
#include <string.h>
#include <zephyr/logging/log.h>
#include "zephyr_async_uart.h"
#include "uart_protocol.h"

#define UART_ALIAS DT_ALIAS(my_serial)
#define RX_BUF_SIZE 128
#define RX_BUF_NUM 4
#define RX_TIMEOUT_US 2000

LOG_MODULE_REGISTER(app_uart, LOG_LEVEL_INF);

struct uart_packet
{
    uint8_t* data;
    size_t len;
};

static const struct device* UartDevice = DEVICE_DT_GET(UART_ALIAS);
static K_SEM_DEFINE(tx_done, 0, 1);
static packets_cb_t user_rx_cb = NULL;
uint8_t Serial_ucAckBuffer[256] = { 0 };

K_MEM_SLAB_DEFINE(uart_rx_slab, RX_BUF_SIZE, RX_BUF_NUM, 4);
K_MSGQ_DEFINE(rx_msgq, sizeof(struct uart_packet), 10, 4);
K_MSGQ_DEFINE(tx_msgq, sizeof(struct uart_packet), 10, 4);

static void uart_event_handler(const struct device* dev,
    struct uart_event* evt,
    void* user_data);
static int AppUartInit(void);
static void AppUartTxThread(void* arg1, void* arg2, void* arg3);
static void AppUartRxThread(void* arg1, void* arg2, void* arg3);

static void uart_event_handler(const struct device* dev,
    struct uart_event* evt,
    void* user_data)
{
    int err;

    switch (evt->type)
    {
        case UART_TX_DONE:
            k_sem_give(&tx_done);
            break;

        case UART_RX_RDY:
            // LOG_INF("RX event! len=%d", evt->data.rx.len);
            uint8_t* rx_data = evt->data.rx.buf + evt->data.rx.offset;
            size_t rx_len = evt->data.rx.len;

            uint8_t* heap_buf = k_malloc(rx_len);
            if (!heap_buf)
            {
                LOG_ERR("Heap alloc failed for RX");
                break;
            }

            memcpy(heap_buf, rx_data, rx_len);

            struct uart_packet pkt = { .data = heap_buf, .len = rx_len };
            k_msgq_put(&rx_msgq, &pkt, K_NO_WAIT);
            break;

        case UART_RX_BUF_REQUEST:
            uint8_t* new_buf;
            err = k_mem_slab_alloc(&uart_rx_slab, (void**)&new_buf, K_NO_WAIT);
            if (err)
            {
                LOG_ERR("Slab alloc failed!");
                break;
            }
            uart_rx_buf_rsp(UartDevice, new_buf, RX_BUF_SIZE);
            break;

        case UART_RX_BUF_RELEASED:
            k_mem_slab_free(&uart_rx_slab, (void*)evt->data.rx_buf.buf);
            break;

        default:
            break;
    }
}

static int AppUartInit(void)
{
    LOG_INF("AppUartInit running");
    // LOG_INF("UartDevice=%p, ready=%d, name=%s", UartDevice, device_is_ready(UartDevice), UartDevice ? UartDevice->name : "NULL");

    struct uart_config cfg;
    uart_config_get(UartDevice, &cfg);
    cfg.baudrate = 921600;
    uart_configure(UartDevice, &cfg);
    setUartCurrentBaud(921600);

    uint8_t* first_buf;
    int err;

    if (!device_is_ready(UartDevice))
    {
        LOG_ERR("UART device not ready");

        return -ENODEV;
    }

    uart_callback_set(UartDevice, uart_event_handler, NULL);

    err = k_mem_slab_alloc(&uart_rx_slab, (void**)&first_buf, K_NO_WAIT);
    if (err)
        return err;

    err = uart_rx_enable(UartDevice, first_buf, RX_BUF_SIZE, RX_TIMEOUT_US);
    if (err)
    {
        k_mem_slab_free(&uart_rx_slab, (void*)first_buf);
        return err;
    }

    return 0;
}

uint8_t Hex2Char(uint8_t c, bool* pbOk)
{
    uint8_t u8Result = 0U;

    if (pbOk != NULL) {
        *pbOk = true;
    }

    if ((c >= 0) && (c <= 9)) {
        u8Result = c + '0';
    }
    else if (c >= 0x0a && c <= 0x0f) {
        u8Result = c + 'A' - 0x0a;
    }
    return u8Result;
}

int UartRxCbRegister(packets_cb_t cb)
{
    if (!cb)
        return -EINVAL;
    user_rx_cb = cb;
    return 0;
}

int SerialSendMsg(const uint8_t* data, size_t len)
{
    if (!data || len == 0)
        return -EINVAL;

    uint8_t* tx_copy = k_malloc(len);
    if (!tx_copy)
        return -ENOMEM;

    memcpy(tx_copy, data, len);

    struct uart_packet pkt = { .data = tx_copy, .len = len };
    return k_msgq_put(&tx_msgq, &pkt, K_NO_WAIT);
}

void SendOnlyNAck(void)
{
    const char* msg = "$DSIMC,NACK*ED\r\n";
    SerialSendMsg((const uint8_t*)msg, strlen(msg));
}

void SendAck(void)
{
    const char* msg = "$DSIMC,ACK*50\r\n";
    SerialSendMsg((const uint8_t*)msg, strlen(msg));
}

void SendERRWriteAck(uint8_t u8Addr, uint8_t rel)
{
    uint8_t crc_ack;
    bool c2h_flag;
    uint8_t u8length;

    sprintf(&Serial_ucAckBuffer[0], "$DSIMC,NACK,%02X,%02X\r\n", u8Addr, rel);
    u8length = strlen(Serial_ucAckBuffer);

    Serial_ucAckBuffer[u8length - 2] = '*';
    crc_ack = CRC_8(Serial_ucAckBuffer, u8length - 2);
    Serial_ucAckBuffer[u8length - 1] = Hex2Char((crc_ack >> 4), &c2h_flag);
    Serial_ucAckBuffer[u8length] = Hex2Char((crc_ack & 0x0F), &c2h_flag);
    sprintf(&Serial_ucAckBuffer[u8length + 1], "\r\n");
    SerialSendMsg(Serial_ucAckBuffer, strlen(Serial_ucAckBuffer));
}

void SendReadWriteAck(uint8_t u8Addr, uint8_t u8Val)
{
    uint8_t crc_ack;
    bool c2h_flag;
    uint8_t u8length;

    sprintf(&Serial_ucAckBuffer[0], "$DSIMC,ACK,%02X,%02X\r\n", u8Addr, u8Val);
    u8length = strlen(Serial_ucAckBuffer);

    Serial_ucAckBuffer[u8length - 2] = '*';
    crc_ack = CRC_8(Serial_ucAckBuffer, u8length - 2);
    Serial_ucAckBuffer[u8length - 1] = Hex2Char((crc_ack >> 4), &c2h_flag);
    Serial_ucAckBuffer[u8length] = Hex2Char((crc_ack & 0x0F), &c2h_flag);
    sprintf(&Serial_ucAckBuffer[u8length + 1], "\r\n");
    SerialSendMsg(Serial_ucAckBuffer, strlen(Serial_ucAckBuffer));
}

int UartReconfig(uint32_t baudrate)
{
    // k_msleep(200);

    struct uart_config cfg;
    int err;

    uart_rx_disable(UartDevice);

    uart_config_get(UartDevice, &cfg);

    cfg.baudrate = baudrate;
    setUartCurrentBaud(baudrate);

    err = uart_configure(UartDevice, &cfg);
    if (err)
    {
        printk("UART reconfig failed: %d\n", err);
        return err;
    }

    uint8_t* buf;
    err = k_mem_slab_alloc(&uart_rx_slab, (void**)&buf, K_NO_WAIT);
    if (err)
    {
        printk("UART buf alloc failed\n");
        return err;
    }

    err = uart_rx_enable(UartDevice, buf, RX_BUF_SIZE, RX_TIMEOUT_US);
    if (err)
    {
        printk("UART rx enable failed\n");
        k_mem_slab_free(&uart_rx_slab, buf);
        return err;
    }

    //printk("UART baudrate changed to %d\n", baudrate);

    return 0;
}


static void AppUartTxThread(void* arg1, void* arg2, void* arg3)
{
    ARG_UNUSED(arg1);
    ARG_UNUSED(arg2);
    ARG_UNUSED(arg3);

    struct uart_packet pkt;

    while (1)
    {
        k_msgq_get(&tx_msgq, &pkt, K_FOREVER);

        int err = uart_tx(UartDevice, pkt.data, pkt.len, SYS_FOREVER_US);
        if (err)
        {
            LOG_ERR("uart_tx failed: %d", err);
            k_free(pkt.data);
            continue;
        }

        k_sem_take(&tx_done, K_FOREVER);

        k_free(pkt.data);
    }
}

static void AppUartRxThread(void* arg1, void* arg2, void* arg3)
{
    ARG_UNUSED(arg1);
    ARG_UNUSED(arg2);
    ARG_UNUSED(arg3);

    struct uart_packet pkt;

    while (1)
    {
        k_msgq_get(&rx_msgq, &pkt, K_FOREVER);

        if (user_rx_cb)
        {
            user_rx_cb(pkt.data, pkt.len);
        }
        else
        {
            k_free(pkt.data);
        }
    }
}

K_THREAD_DEFINE(tx_tid, 1024, AppUartTxThread, NULL, NULL, NULL, 5, 0, 0);
K_THREAD_DEFINE(rx_tid, 1024, AppUartRxThread, NULL, NULL, NULL, 5, 0, 0);

SYS_INIT(AppUartInit, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);
