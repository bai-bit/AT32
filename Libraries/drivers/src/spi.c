#include<spi.h>
SPI_Type *SPI_list[] = {SPI1,SPI2,SPI3,SPI4};
void spi_init(uint8_t SPIx,uint32_t baud)
{
    //首先配置为缺省值。全是0
	//然后根据自己的需要配寄存器
	//配置的内容：数据的传输方向，全双工，8位数据帧，lsb的格式帧，分频系数256，主机模式，cpol = 0，cpha = 1，nss值，crc
	//使能spi的时钟
	//最后使能spi
    if(SPIx == HW_SPI1)
        RCC->APB2EN |= RCC_APB2EN_SPI1EN;
    else if(SPIx == HW_SPI2)
        RCC->APB1EN |= RCC_APB1EN_SPI2EN;
    else if(SPIx == HW_SPI3)
        RCC->APB1EN |= RCC_APB1EN_SPI3EN;
    else if(SPIx == HW_SPI4)
        RCC->APB1EN |= RCC_APB1EN_SPI4EN;

    SPI_list[SPIx]->CTRL2 &= ~SPI_CTRL2_MCLKP_3;
    SPI_list[SPIx]->CTRL1 &= SPI_CR1_Mask;

    SPI_list[SPIx]->CTRL1 |= (SPI_CTRL1_MSTEN | SPI_CTRL1_ISS  | SPI_CTRL1_SWNSSEN | baud);
    SPI_list[SPIx]->CTRL1 &=  ~(SPI_CTRL1_LSBEN);

    SPI_list[SPIx]->I2SCTRL &= SPI_MODE_SEL;
    SPI_list[SPIx]->CPOLY = SPI_CPOLY_RESET;
    
    spi_cmd(SPIx, ENABLE);
}
void spi_resetMode(uint8_t SPIx,uint8_t mode)
{
    //MSTEN
    while(spi_GetFlagStatus(SPIx,SPI_Flag_BSY) == SET)
        continue;
    spi_cmd(SPIx,DISABLE);
    SPI_list[SPIx]->CTRL1 &= ~(SPI_CTRL1_MSTEN);
    SPI_list[SPIx]->CTRL1 |= mode;
    spi_cmd(SPIx,ENABLE);
}
void spi_resetTR(uint8_t SPIx,uint8_t mode)
{
    while(spi_GetFlagStatus(SPIx,SPI_Flag_BSY) == SET)
        continue;
    spi_cmd(SPIx,DISABLE);
    SPI_list[SPIx]->CTRL1 &= ~(SPI_CTRL1_CPHA | SPI_CTRL1_CPOL);
    switch(mode)
    {
        case MODE0:
            break;
        case MODE1:
            SPI_list[SPIx]->CTRL1 |= (SPI_CTRL1_CPOL);
            break;
        case MODE2:
            SPI_list[SPIx]->CTRL1 |= (SPI_CTRL1_CPHA);
            break;
        case MODE3:
            SPI_list[SPIx]->CTRL1 |= (SPI_CTRL1_CPHA | SPI_CTRL1_CPOL);
            break;
        default:
            printf("reset TR error");
    }
 
    spi_cmd(SPIx,ENABLE);
}
void spi_resetTransmode(uint8_t SPIx,uint8_t Transmode)
{
    while(spi_GetFlagStatus(SPIx,SPI_Flag_BSY) == SET)
        continue;
    spi_cmd(SPIx,DISABLE);
    SPI_list[SPIx]->CTRL1 &=~(SPI_CTRL1_LSBEN);
    SPI_list[SPIx]->CTRL1 |= Transmode;
    spi_cmd(SPIx,ENABLE);
}
void spi_resetFirstBits(uint8_t SPIx,uint8_t FirstBits)
{
    while(spi_GetFlagStatus(SPIx,SPI_Flag_BSY) == SET)
        continue;
    spi_cmd(SPIx,DISABLE);
    SPI_list[SPIx]->CTRL1 &= ~(SPI_CTRL1_DFF16);
    SPI_list[SPIx]->CTRL1 |= FirstBits;
    spi_cmd(SPIx,ENABLE);
}
void spi_cmd(uint8_t SPIx,FunctionalState status)
{
    if(status == ENABLE)
        SPI_list[SPIx]->CTRL1 |= SPI_CTRL1_SPIEN;
    else
        SPI_list[SPIx]->CTRL1 &= ~SPI_CTRL1_SPIEN;
}

FlagStatus spi_GetFlagStatus(uint8_t SPIx,uint16_t SPI_FLAG)
{
	if(SPI_list[SPIx]->STS & SPI_FLAG)
		return SET;
	else
		return RESET;
}

//add baud reset functional achieve
void spi_resetbaud(uint8_t SPIx,uint32_t baud)
{
    while(spi_GetFlagStatus(SPIx,SPI_Flag_BSY) == SET)
        continue;
    spi_cmd(SPIx,DISABLE);
    SPI_list[SPIx]->CTRL1 &= SPI_BAUDRATEMask;
	if(baud == SPI_BAUDRATE_512 )
		SPI_list[SPIx]->CTRL2 |= baud;
	if(baud == SPI_BAUDRATE_1024)
	{
		SPI_list[SPIx]->CTRL1 |= SPI_BAUDRATE_4;
		SPI_list[SPIx]->CTRL2 |= SPI_BAUDRATE_512;
	}
	else
		SPI_list[SPIx]->CTRL1 |= baud;
    spi_cmd(SPIx,ENABLE);
}
//add operation spi->DT register function
void spi_senddata(uint8_t SPIx,u8 data)
{
    SPI_list[SPIx]->DT = data;
}
u8 spi_receivedata(uint8_t SPIx)
{
    return SPI_list[SPIx]->DT;
}
//add spi read_write functional achieve
u8 spi_RWdata(uint8_t SPIx,u8 data)
{
    uint32_t time = 0x10000;
    
    while(spi_GetFlagStatus(SPIx,SPI_Flag_BSY) == SET && time--)
        continue;
    time = 0x10000;
    spi_senddata(SPIx,data);
    while(spi_GetFlagStatus(SPIx,SPI_Flag_BSY) == SET && time--)
        continue;
    
    return spi_receivedata(SPIx);
}


