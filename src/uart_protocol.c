#include "uart_protocol.h"
#include <stdio.h>
#include <string.h>
#include "zephyr_systick.h"
#include "DsImuSdk.h"

uint8_t u8count;
uint8_t u8bincount;
uint8_t crc_ack;
int16_t i16temp;

uint8_t CRC_8(uint8_t* buf, uint16_t len)
{
    uint8_t crc = 0;

    for (uint16_t i = 0; i < len; i++)
    {
        crc ^= buf[i];
        for (uint8_t j = 0; j < 8; j++)
        {
            if (crc & 0x80)
                crc = (crc << 1) ^ 0x07;
            else
                crc <<= 1;
        }
    }
    return crc;
}

int Composer_PrintImu(unsigned char* pStr, DsImu_ImuCorrectedDataType* pxSendData)
{
    uint8_t u8binhead[3] = { 0XA5, 0X5A, 0X00 };
    uint8_t u8end[2] = { 0X0D, 0X0A };
    uint64_t u64UnixTime;
    uint8_t u8cmdlen[2] = { 0X10, 0X40 };
    uint8_t idx = 0;
    int16_t i16temp;
    uint8_t crc_ack;

    if (u8bincount < 255) {
        u8bincount++;
    }
    else {
        u8bincount = 0;
    }

    memcpy(pStr, u8binhead, 3);
    idx += 3;

    memcpy(pStr + idx, u8cmdlen, 2);
    idx += 2;

    memcpy(pStr + idx, &u8bincount, 1);
    idx += 1;

    u64UnixTime = (uint64_t)k_uptime_get();
    // u64UnixTime = GetTickUs(); 
    memcpy(pStr + idx, &u64UnixTime, 8);
    idx += 8;

    memcpy(pStr + idx, pxSendData->fAlignedAcceleration, 12);
    idx += 12;

    memcpy(pStr + idx, pxSendData->fAlignedAngularRate, 12);
    idx += 12;

    memcpy(pStr + idx, &pxSendData->fAttitude[1], 4);
    idx += 4;
    memcpy(pStr + idx, &pxSendData->fAttitude[2], 4);
    idx += 4;
    memcpy(pStr + idx, &pxSendData->fAttitude[0], 4);
    idx += 4;

    memcpy(pStr + idx, pxSendData->fQuat, 16);
    idx += 16;

    i16temp = 10 * pxSendData->fTemperature;
    memcpy(pStr + idx, &i16temp, 2);
    idx += 2;

    *(pStr + idx) = pxSendData->u16AhrsStatus & 0xFFU;
    idx += 1;

    crc_ack = CRC_8(pStr, idx);
    memcpy(pStr + idx, &crc_ack, 1);
    idx += 1;

    memcpy(pStr + idx, u8end, 2);
    idx += 2;

    return idx;
}
