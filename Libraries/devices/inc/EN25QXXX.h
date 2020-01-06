#ifndef __EN25QXXX_H
#define __EN25QXXX_H

#include<string.h>
#include<stdint.h>

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
    uint8_t channel;
    uint8_t (*xfer_data)(uint8_t data);
    
    void (*cs)(uint8_t status);
}EN25Q_dev_t;

void EN25Q_module_init(EN25Q_dev_t *opers);
uint16_t EN25QXXX_init(void);
static void EN25QXXX_active_mode(void);
static void EN25QXXX_wait_busy(void);

void EN25QXXX_write_nocheck(uint32_t addr,uint8_t *buf,uint32_t bufnum);
uint32_t EN25QXXX_write_data(uint32_t addr,uint8_t *buf,uint32_t bufnum);
uint32_t EN25QXXX_write_page(uint32_t addr,uint8_t *buf,uint32_t bufnum);

uint16_t EN25QXXX_read_id(void);
uint32_t EN25QXXX_read_data(uint32_t addr,uint8_t *buf,uint32_t bufnum);
static uint8_t EN25QXXX_read_register(void);

void EN25QXXX_erase_sector(const uint32_t addr);
void EN25QXXX_sleep_mode(void);

#endif
