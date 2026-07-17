#include "zephyr_spi_hal.h"

#define SPI_NODE DT_NODELABEL(spi2)
static const struct device* spi_dev = DEVICE_DT_GET(SPI_NODE);
static const struct device* gpio_b = DEVICE_DT_GET(DT_NODELABEL(gpiob));

#if defined(IMU_TARGET_IM3J)
#define CS_ACCEL_PIN 9 // PB9
#define CS_GYRO_PIN 8 // PB8
#define CS_EEPROM_PIN 4 // PB4

#elif defined(IMU_TARGET_IM8S)
#define CS_IMU_PIN 9 // PB9
#define CS_EEPROM_PIN 4 // PB4
#endif

static struct spi_config spi_cfg = {
	.frequency = 1000000,
	.operation = SPI_OP_MODE_MASTER |
				 SPI_TRANSFER_MSB |
				 SPI_WORD_SET(8),

	.slave = 0,
	.cs = NULL,
};

#define MAX_SPI_BUF_SIZE 256
__aligned(4) static uint8_t tx_buffer[MAX_SPI_BUF_SIZE];
__aligned(4) static uint8_t rx_buffer[MAX_SPI_BUF_SIZE];

void cs_select(DsImuChipId_t eChipId)
{

#if defined(IMU_TARGET_IM3J)
	switch (eChipId) {
		case DS_IM3J_ACC:
			gpio_pin_set(gpio_b, CS_ACCEL_PIN, 0);
			break;
		case DS_IM3J_GYR:
			gpio_pin_set(gpio_b, CS_GYRO_PIN, 0);
			break;
		case DS_EEPROM:
			gpio_pin_set(gpio_b, CS_EEPROM_PIN, 0);
			break;
		default:
			break;
	}

#elif defined(IMU_TARGET_IM8S)
	switch (eChipId) {
		case DS_IM8S_IMU:
			gpio_pin_set(gpio_b, CS_IMU_PIN, 0);
			break;
		case DS_EEPROM:
			gpio_pin_set(gpio_b, CS_EEPROM_PIN, 0);
			break;
		default:
			break;
	}
#endif

}

void cs_deselect(DsImuChipId_t eChipId)
{
#if defined(IMU_TARGET_IM3J)
	switch (eChipId) {
		case DS_IM3J_ACC:
			gpio_pin_set(gpio_b, CS_ACCEL_PIN, 1);
			break;
		case DS_IM3J_GYR:
			gpio_pin_set(gpio_b, CS_GYRO_PIN, 1);
			break;
		case DS_EEPROM:
			gpio_pin_set(gpio_b, CS_EEPROM_PIN, 1);
			break;
		default:
			break;
	}

#elif defined(IMU_TARGET_IM8S)
	switch (eChipId) {
		case DS_IM8S_IMU:
			gpio_pin_set(gpio_b, CS_IMU_PIN, 1);
			break;
		case DS_EEPROM:
			gpio_pin_set(gpio_b, CS_EEPROM_PIN, 1);
			break;
		default:
			break;
	}
#endif

}

uint8_t spiTransfer(DsImuChipId_t eChipId,
	DsImu_SpiBuffType* pxSpiBufArray,
	uint8_t u8ArraySize)
{
	if (!pxSpiBufArray || u8ArraySize == 0)
		return 1;
	for (int j = 0; j < pxSpiBufArray[0].u16Len && j < 16; j++)
		cs_select(eChipId);
	k_usleep(10);

	for (uint8_t i = 0; i < u8ArraySize; i++)
	{
		uint16_t len = pxSpiBufArray[i].u16Len;

		if (len == 0 || len > MAX_SPI_BUF_SIZE)
		{
			cs_deselect(eChipId);
			return 1;
		}

		memcpy(tx_buffer, pxSpiBufArray[i].pTxBuf, len);

		struct spi_buf tx_buf = {
			.buf = tx_buffer,
			.len = len };

		struct spi_buf rx_buf = {
			.buf = rx_buffer,
			.len = len };

		struct spi_buf_set tx_set = {
			.buffers = &tx_buf,
			.count = 1 };

		struct spi_buf_set rx_set = {
			.buffers = &rx_buf,
			.count = 1 };

		int ret = spi_transceive(spi_dev, &spi_cfg, &tx_set, &rx_set);
		if (ret != 0)
		{
			cs_deselect(eChipId);
			return 1;
		}
		memcpy(pxSpiBufArray[i].pRxBuf, rx_buffer, len);
	}

	cs_deselect(eChipId);

	return 0;
}

void spiCancel(DsImuChipId_t eChipId)
{
	cs_deselect(eChipId);
}

uint8_t spiGetTransferResult(DsImuChipId_t eChipId)
{
	return 0;
}

int Zephyr_SPI_GPIO_Init(void)
{
	if (!device_is_ready(gpio_b))
	{
		printk("GPIO not ready\n");
		return -1;
	}

#if defined(IMU_TARGET_IM3J)
	gpio_pin_configure(gpio_b, CS_ACCEL_PIN, GPIO_OUTPUT_HIGH);
	gpio_pin_configure(gpio_b, CS_GYRO_PIN, GPIO_OUTPUT_HIGH);
	gpio_pin_configure(gpio_b, CS_EEPROM_PIN, GPIO_OUTPUT_HIGH);

#elif defined(IMU_TARGET_IM8S)
	gpio_pin_configure(gpio_b, CS_IMU_PIN, GPIO_OUTPUT_HIGH);
	gpio_pin_configure(gpio_b, CS_EEPROM_PIN, GPIO_OUTPUT_HIGH);
#endif

	return 0;
}
