#ifndef __SPI_FLASH_H
#define __SPI_FLASH_H
#include "at32f4xx.h"
#include <stdio.h>


#define TIMEOUT                 0x01000000
#define FLASH_KEY1              0x45670123
#define FLASH_KEY2              0xCDEF89AB
#define SECTOR_SIZE             2048
#define BANK1                   255
#define FLASH_SIZE              1024	
#define FLASH_FLAG_BSY          1  
#define FLASH_FLAG_PRCDN        0x0020
#define FLASH_FLAG_PRGMFLR      0x0004
#define FLASH_FLAG_WRPRTFLR     0x0010

#define CTRL_PRGM_SET           0x0001
#define CTRL_PGERS_SET          0x0002
#define CTRL_RSTR_SET           0x0040

typedef enum
{
    FLASH_BSY = 1,
    FLASH_PRGMFLR,
    FLASH_WRPRTFLR,
    FLASH_COMPLETE,
    FLASH_TIMEOUT
} FLASH_Status;

void flash_init(void);
void flash_lock(void);

uint16_t Flash_ReadHalfWord(uint32_t addr);		    

void Flash_Write(uint32_t addr,uint16_t *buf,uint16_t len);		//从指定地址开始写入指定长度的数据
void flash_read(uint32_t addr,uint16_t *buf,uint16_t len);   		//从指定地址开始读出指定长度的数据
void Flash_WritePageWord(uint32_t addr,uint16_t *buf,uint16_t len);

FLASH_Status Flash_WriteHalfWord(uint32_t addr, uint16_t data);
FLASH_Status Flash_WaitProcess(uint32_t timeout);
FLASH_Status Flash_GetStatus(void);
FLASH_Status Flash_ErasePage(uint32_t addr);

#endif
