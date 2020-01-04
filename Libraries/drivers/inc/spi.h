#ifndef __SPI_H
#define __SPI_H
#include<at32f4xx.h>
#include<stdio.h>

#define SPI_CR1_Mask            (0xE9F3)
#define SPI_MODE_SEL            0xF7FF

#define SPI_Flag_TX             0x0002
#define SPI_Flag_BSY            (1 << 7)

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

#define SPI_CPOLY_RESET         0x0007

#define MODE0                   0
#define MODE1                   1
#define MODE2                   2
#define MODE3                   3

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


void spi_init(SPI_Type *SPIx,uint32_t baud);
FlagStatus spi_GetFlagStatus(SPI_Type *SPIx,uint16_t SPI_FLAG);

void spi_cmd(SPI_Type *SPIx,FunctionalState status);
void spi_senddata(SPI_Type *SPIx,u8 data);
u8 spi_receivedata(SPI_Type *SPIx);
u8 spi_RWdata(SPI_Type *SPIx,u8 data);

void spi_resetTR(SPI_Type *SPIx,uint8_t mode);
void spi_resetMode(SPI_Type *SPIx,uint8_t mode);
void spi_resetbaud(SPI_Type *SPIx,uint32_t baud);
void spi_resetTransmode(SPI_Type *SPIx,uint8_t Transmode);
void spi_resetFirstBits(SPI_Type *SPIx,uint8_t FirstBits);

#endif
