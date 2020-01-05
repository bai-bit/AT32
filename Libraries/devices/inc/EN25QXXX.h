#ifndef __EN25QXXX_H
#define __EN25QXXX_H
#include<at32f4xx.h>
#include<stdlib.h>
#include<string.h>

#define SECSIZE              4096
#define SECNUM               4096
#define PAGESIZE             256

#define SEND_DEFAULT_VALUE   0xFF
#define READ_DEVICE_ID       0x90
#define READ_IDENTIFICATION  0x9F
#define EN25Q_RELEASE_SLEEP  0xAB
#define EN25Q_DEEP_POWER     0xB9
#define EN25Q_FAST_READ      0x03
#define EN25Q_READ_STATUS    0x05
#define EN25Q_WRITE_ENABLE   0x06
#define EN25Q_WRITE_DISABLE  0x04
#define EN25Q_WRITE_STATUS   0x01
#define EN25Q_PAGE_PROGRAM   0x02
#define EN25Q_SECTOR_ERASE   0x20
#define EN25Q_64K_ERASE      0x08
#define EN25Q_CHIP_ERASE     0x60
#define EN25Q_BASE           0X02
typedef enum {ID,STATUS,DATA,SECTOR,CHIP}cmd;

typedef struct EN25Q_dev_t {
    uint8_t (*msg_que)(SPI_Type *SPIx,uint8_t data);
   
    void (*cs)(uint32_t GPIOx,uint16_t Pin,uint8_t num);
    uint32_t GPIOx;
    uint16_t Pin;
    
}EN25Q_dev_t;

void EN25Q_module_init(EN25Q_dev_t *opers);
uint16_t EN25QXXX_init(SPI_Type *SPIx);
void EN25Q_module_init(EN25Q_dev_t *operations);
void EN25QXXX_active_mode(SPI_Type *SPIx);
void EN25QXXX_wait_busy(SPI_Type *SPIx);

uint32_t EN25QXXX_write(SPI_Type *SPIx,uint32_t addr,uint8_t *buf,uint32_t num,uint8_t cmd);
void EN25QXXX_write_nocheck(SPI_Type *SPIx,uint32_t addr,u8 *buf,uint32_t bufnum);
uint32_t EN25QXXX_write_data(SPI_Type *SPIx,uint32_t addr,u8 *buf,uint32_t bufnum);
void EN25QXXX_write_page(SPI_Type *SPIx,const uint32_t addr,u8 *buf,uint16_t bufnum);

uint32_t EN25QXXX_read(SPI_Type *SPIx,uint32_t addr,uint8_t *rbuf,uint32_t num,uint8_t cmd);
uint16_t EN25QXXX_readID(SPI_Type *SPIx);
uint32_t EN25QXXX_read_data(SPI_Type *SPIx,const uint32_t addr,u8 *buf,uint32_t num);
uint8_t EN25QXXX_read_register(SPI_Type *SPIx);

void EN25QXXX_clear(SPI_Type *SPIx,uint32_t addr,uint8_t cmd);
void EN25QXXX_erase_sector(SPI_Type *SPIx,const uint32_t addr);
void EN25QXXX_close(SPI_Type *SPIx);
void EN25QXXX_sleep_mode(SPI_Type *SPIx);
#endif
