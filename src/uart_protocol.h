#ifndef UART_PROTOCOL_H_
#define UART_PROTOCOL_H_

#include <stdint.h>
#include <stdbool.h>
#include "DsImuSdk.h"

int Composer_PrintImu(unsigned char* pStr, DsImu_ImuCorrectedDataType* pxSendData);
uint8_t CRC_8(uint8_t* buf, uint16_t len);
void setUartSendPeriodUs(const uint32_t u32UartSendPeriodUs);
uint32_t GetUartSendPeriodUs(void);

void setUartBaud(const uint32_t u32UartBaud);
void setUartCurrentBaud(const uint32_t u32UartBaud);
uint32_t GetUartBaud(void);
uint32_t GetUartCurrentBaud(void);

#endif /* Serial_H_ */
