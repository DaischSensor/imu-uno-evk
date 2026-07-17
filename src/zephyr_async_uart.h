#ifndef __ZEPHYR_ASYNC_UART_H
#define __ZEPHYR_ASYNC_UART_H

#include <stddef.h>
#include <stdint.h>

#define SERIAL_CMD_BUFF_SIZE    (256)
#define SERIAL_SEND_BUFFER_SIZE (256)
#define SERIAL_CMD_ADD_MAX      (3)
#define SERIAL_CMD_ADD_ERR      (4)

#ifdef __cplusplus
extern "C" {
#endif

    typedef void (*packets_cb_t)(uint8_t* data, size_t len);

    int UartRxCbRegister(packets_cb_t cb);
    int SerialSendMsg(const uint8_t* data, size_t len);

    extern uint8_t Serial_ucAckBuffer[SERIAL_CMD_BUFF_SIZE];


    uint8_t Hex2Char(uint8_t c, bool* pbOk);
    extern bool gImuOutputEnabled;
    extern bool gImuReinitFlag;
    extern bool gImuSerialOutputEnabled;


    void SendOnlyNAck(void);
    void SendAck(void);
    void SendERRWriteAck(uint8_t u8Addr, uint8_t rel);
    void SendReadWriteAck(uint8_t u8Addr, uint8_t u8Val);
    bool CheckSetCmdValidity(const uint8_t* pBuffer, uint32_t rxBytes,
        uint8_t* pAddrOfs, uint8_t* pRegVal);
    bool ProcessDSIMCCommand(uint8_t* pBuffer, uint32_t rxBytes);
    bool ProcessSerialBuffer(uint8_t* pBuffer, uint32_t* pRxBytes);
    int UartReconfig(uint32_t baudrate);




#ifdef __cplusplus
}
#endif

#endif
