#ifndef __EN25QXXX_H
#define __EN25QXXX_H
#include<at32f4xx.h>
#include<gpio_init.h>
#include<spi.h>
#include<stdio.h>
#include<systick.h>

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


void EN25QXXX_init(SPI_Type *SPIx);
uint16_t EN25QXXX_readID(SPI_Type *SPIx);
void spim_init(SPI_Type *SPIx);
void EN25QXXX_active_mode(void);
u8 *EN25QXXX_read_data(uint32_t addr,u8 *buf,uint32_t num);
void EN25QXXX_wait_busy(void);
u8 EN25QXXX_read_register(void);
void EN25QXXX_erase_sector(uint32_t addr);
void EN25QXXX_write_nocheck(uint32_t addr,u8 *buf,uint32_t bufnum);
void EN25QXXX_write_data(uint32_t addr,u8 *buf,uint32_t bufnum);

#endif
