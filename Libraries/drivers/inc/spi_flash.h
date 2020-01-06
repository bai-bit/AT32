//在这里实现spi_flash的操作逻辑
#ifndef __SPI_FLASH_H
#define __SPI_FLASH_H
#include<string.h>
#include<stdint.h>

#define SECSIZE     4096
#define PAGESIZE    256
typedef struct spi_flash_dev_t {
    uint16_t device_id;
    uint16_t (*open)(void) ;
	uint32_t (*read)(uint32_t addr, uint8_t *buf, uint32_t bufnum);
	uint32_t (*write)(uint32_t addr, uint8_t *buf, uint32_t bufnum);
	void (*erase)(uint32_t addr);
	void (*release)(void);
}spi_flash_dev_t;

typedef struct EN25Q_dev_t {
    uint8_t channel;
    uint8_t (*xfer_data)(uint8_t data);
    
    void (*cs)(uint8_t status);
}EN25Q_dev_t;

void EN25Q_module_init(EN25Q_dev_t *opers);
void spi_flash_module_init(spi_flash_dev_t *operation);
uint16_t spi_flash_open(void);
uint32_t spi_flash_read(uint32_t addr, uint8_t *buf, uint32_t bufnum);
uint32_t spi_flash_write(uint32_t addr, uint8_t *buf, uint32_t bufnum);
void spi_flash_erase(uint32_t addr);


#endif
