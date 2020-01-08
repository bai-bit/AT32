#ifndef __SPI_FLASH_H
#define __SPI_FLASH_H
#include<string.h>
#include<stdint.h>

typedef struct spi_flash_dev_t {
    uint16_t device_id;
    uint8_t (*xfer_data)(uint8_t data);
    void (*cs)(uint8_t status);
}spi_flash_dev_t;

void spi_flash_init(spi_flash_dev_t *ops);
uint16_t spi_flash_open(void);
uint32_t spi_flash_read(uint32_t addr, uint8_t *buf, uint32_t len);
uint32_t spi_flash_write(uint32_t addr, uint8_t *buf, uint32_t len);
void spi_flash_erase(uint32_t addr);


#endif
