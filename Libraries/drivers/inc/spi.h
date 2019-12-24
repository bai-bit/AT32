#ifndef __SPI_H
#define __SPI_H
#include<at32f4xx.h>
#include<gpio_init.h>

#define SPI_FULLDUPLEX          0x0000
#define SPI_WORKMODE_Master     0x0104
#define SPI_DATASIZE_8Bits      0x0000
#define SPI_CPOL_LOW            0x00
#define SPI_CPOL_HIGH           0X02
#define SPI_CPHA_1EDGE          0X00
#define SPI_CPHA_2EDGE          0x01
#define SPI_NSSSEL_SOFT         0x0200

#define SPI_FRISTBIT_LSB        0x0080
#define SPI_FRISTBIT_MSB        0x0000
#define SPI_CRCPOLYNOMIAL       (0x7)
#define SPI_FristBit_Mask       (~(1 << 7))
#define SPI_CR1_Mask            (0xE9F3)
#define SPI_MODE_SEL            0xF7FF
#define SPI1EN_CLK              0x0400
#define SPI1EN                  0x0040
#define SPI_Flag_TX             0x0002
#define SPI_Flag_BSY            (1 << 7)
#define CTRL1_SPIEN_SET         (0x0040)

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


void spi_init(SPI_Type *SPIx,SPI_InitType *SPI_InitStruct);
FlagStatus spi_GetFlagStatus(SPI_Type *SPIx,uint16_t SPI_FLAG);
void spi_resetbaud(SPI_Type *SPIx,uint32_t baud);
void spi_cmd(SPI_Type *SPIx,FunctionalState status);
void spi_senddata(SPI_Type *SPIx,u8 data);
u8 spi_receivedata(SPI_Type *SPIx);
u8 spi_RWdata(SPI_Type *SPIx,u8 data);

#endif
