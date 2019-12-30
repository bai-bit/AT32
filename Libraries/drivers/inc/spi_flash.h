//在这里实现spi_flash的操作逻辑
#ifndef __SPI_FLASH_H
#define __SPI_FLASH_H
#include<at32f4xx.h>
#include<string.h>

typedef struct spi_flash_dev_t {
    void (*init)(SPI_Type *SPIx,void *SPI_InitStruct);
	uint16_t (*open)(SPI_Type *SPIx);
	uint32_t (*read)(SPI_Type *SPIx,uint32_t addr,uint8_t *buf,uint32_t bufnum,uint8_t cmd);
	uint32_t (*write)(SPI_Type *SPIx,uint32_t addr,uint8_t *buf,uint32_t bufnum);
	void (*clear)(SPI_Type *SPIx,uint32_t addr,uint8_t cmd);
	void (*release)(SPI_Type *SPIx);
    void (*resetbaud)(SPI_Type *SPIx,uint32_t baud);
}spi_flash_dev_t;

uint16_t spi_flash_open(SPI_Type *SPIx,spi_flash_dev_t *operation);
uint32_t spi_flash_read(SPI_Type *SPIx,uint32_t addr,uint8_t *buf,uint32_t bufnum,uint8_t cmd);
uint32_t spi_flash_write(SPI_Type *SPIx,uint32_t addr,uint8_t *buf,uint32_t bufnum,uint8_t cmd);

void spi_flash_clear(SPI_Type *SPIx,uint32_t addr,uint8_t cmd);

#endif
