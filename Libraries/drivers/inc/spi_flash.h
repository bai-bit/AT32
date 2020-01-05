//在这里实现spi_flash的操作逻辑
#ifndef __SPI_FLASH_H
#define __SPI_FLASH_H
#include<at32f4xx.h>
#include<string.h>

#define SECSIZE     4096
#define PAGESIZE    256
typedef struct spi_flash_dev_t {
    uint16_t (*open)(void);
	uint32_t (*read)(uint32_t addr,uint8_t *buf,uint32_t bufnum);
	uint32_t (*write)(uint32_t addr,uint8_t *buf,uint32_t bufnum);
	void (*clear)(uint32_t addr);
	void (*release)(void);
    void (*resetbaud)(uint32_t baud);
    
}spi_flash_dev_t;

uint16_t spi_flash_open(spi_flash_dev_t *operation);
uint32_t spi_flash_read(uint32_t addr,uint8_t *buf,uint32_t bufnum);
uint32_t spi_flash_write(uint32_t addr,uint8_t *buf,uint32_t bufnum);

void spi_flash_clear(uint32_t addr);

#endif
