#ifndef __SPI_H
#define __SPI_H
#include"at32f4xx.h"
#include<stdio.h>

#define SPI_BAUDRATEMask        0xFFC7
#define SPI_BAUDRATE_2          0x0000
#define SPI_BAUDRATE_4          0x0008
#define SPI_BAUDRATE_8          0x0010
#define SPI_BAUDRATE_16         0x0018
#define SPI_BAUDRATE_32         0x0020
#define SPI_BAUDRATE_64         0x0028
#define SPI_BAUDRATE_128        0x0030
#define SPI_BAUDRATE_256        0x0038
#define SPI_BAUDRATE_512        0x0100
#define SPI_BAUDRATE_1024       0x0108

#define MODE0                   (0)
#define MODE1                   (1)
#define MODE2                   (2)
#define MODE3                   (3)
#define HW_SPI1                 (0)
#define HW_SPI2                 (1)
#define HW_SPI3                 (2)
#define HW_SPI4                 (3)

typedef struct 
{
	uint16_t SPI_Transmode;
	uint16_t SPI_Mode;
	uint16_t SPI_FrameSize;
	uint16_t SPI_CPOL;
	uint16_t SPI_CPHA;
	uint16_t SPI_NSSSET;
	uint16_t SPI_MCL;
	uint16_t SPI_FirstBit;
	uint16_t SPI_CPOLY;
}SPI_InitType;
extern SPI_Type *SPI_list[];

void SPI_Init(uint8_t SPIx, uint32_t baud);
FlagStatus SPI_GetFlagStatus(uint8_t SPIx, uint16_t SPI_FLAG);

void SPI_Cmd(uint8_t SPIx, FunctionalState status);
void SPI_SendData(uint8_t SPIx, uint8_t data);
uint8_t SPI_ReceiveData(uint8_t SPIx);
uint8_t SPI_TransferData(uint8_t SPIx, uint8_t data);

void SPI_SetTR(uint8_t SPIx, uint8_t mode);
void SPI_SetMode(uint8_t SPIx, uint8_t mode);
void SPI_SetBaud(uint8_t SPIx, uint32_t baud);
void SPI_SetTransMode(uint8_t SPIx, uint8_t Transmode);
void SPI_SetFirstBits(uint8_t SPIx, uint8_t FirstBits);

#endif
