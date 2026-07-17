#ifndef ZEPHYR_SPI_HAL_H_
#define ZEPHYR_SPI_HAL_H_

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>
#include <string.h>
#include "DsImuSdk.h"

void cs_select(DsImuChipId_t eChipId);
void cs_deselect(DsImuChipId_t eChipId);

uint8_t spiTransfer(DsImuChipId_t eChipId, DsImu_SpiBuffType* pxSpiBufArray, uint8_t u8ArraySize);
void spiCancel(DsImuChipId_t eChipId);
uint8_t spiGetTransferResult(DsImuChipId_t eChipId);

int Zephyr_SPI_GPIO_Init(void);

#endif