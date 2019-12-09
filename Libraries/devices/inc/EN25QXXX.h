#ifndef __EN25QXXX_H
#define __EN25QXXX_H
#include<at32f4xx.h>
#include<gpio_init.h>
#include<spi.h>
#include<stdio.h>
#include<systick.h>

#define READ_DEVICE_ID       0x90
#define READ_IDENTIFICATION  0x9F
#define EN25Q_RELEASE_SLEEP  0xAB

void EN25QXXX_init(SPI_Type *SPIx);
uint16_t EN25QXXX_readID(SPI_Type *SPIx);
void spim_init(SPI_Type *SPIx);
void EN25QXXX_active_mode(void);

#endif
