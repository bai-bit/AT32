#include<spi.h>

void spi1_init(void)
{
	GPIO_Init(HW_GPIOA, gpio_pin_5, gpio_speed_50MHz, mode_af_pp);
	GPIO_Init(HW_GPIOA, gpio_pin_7, gpio_speed_50MHz, mode_af_pp);
	GPIO_Init(HW_GPIOA, gpio_pin_6, gpio_speed_inno,mode_in_pd);

	spi_init(SPI1);
	
}


void spi_init(SPI_Type *SPIx)
{
    //首先配置为缺省值。全是0
	//然后根据自己的需要配寄存器
	//配置的内容：数据的传输方向，全双工，8位数据帧，lsb的格式帧，分频系数256，主机模式，cpol = 0，cpha = 1，nss值，crc
	//使能spi的时钟
	//最后使能spi
    RCC->APB2EN |= 1 << 12;
	SPIx->CTRL2 &= ~SPI_CTRL2_MCLKP_3;
	SPIx->CTRL1 &= SPI_CR1_Mask;
	SPIx->CTRL1 |= (SPI_FULLDUPLEX | SPI_WORKMODE_Master | SPI_DATASIZE_8Bits | SPI_CPOL_HIGH | SPI_CPHA_2EDGE | SPI_NSSSEL_SOFT | SPI_BAUDRATE_256 );
    SPIx->CTRL1 &=  SPI_FRISTBIT_MSB;
    SPIx->I2SCTRL &= SPI_MODE_SEL;
    SPIx->CPOLY = 7;
    spi_cmd(SPIx, ENABLE);
}

void spi_cmd(SPI_Type *SPIx,FunctionalState status)
{
    if(status == ENABLE)
        SPIx->CTRL1 |= CTRL1_SPIEN_SET;
    else
        SPIx->CTRL1 &= ~CTRL1_SPIEN_SET;
}

FlagStatus spi_GetFlagStatus(SPI_Type *SPIx,uint16_t SPI_FLAG)
{
	if(SPIx->STS & SPI_FLAG)
		return SET;
	else
		return RESET;
}

//add baud reset functional achieve
void spi_resetbaud(SPI_Type *SPIx,uint32_t baud)
{
    while(spi_GetFlagStatus(SPIx,SPI_Flag_BSY) == SET)
        continue;
    spi_cmd(SPIx,DISABLE);
    SPIx->CTRL1 &= SPI_BAUDRATEMask;
    SPIx->CTRL1 |= baud;
    spi_cmd(SPIx,ENABLE);
}
//add operation spi->DT register function
void spi_senddata(SPI_Type *SPIx,u8 data)
{
    SPIx->DT = data;
}
u8 spi_receivedata(SPI_Type *SPIx)
{
    return SPIx->DT;
}
//add spi read_write functional achieve
u8 spi_RWdata(SPI_Type *SPIx,u8 data)
{
    uint32_t time = 0x10000;
    
    while(spi_GetFlagStatus(SPIx,SPI_Flag_BSY) == SET )//&& time--)
        continue;
    time = 0x10000;
    spi_senddata(SPIx,data);
    while(spi_GetFlagStatus(SPIx,SPI_Flag_BSY) == SET && time--)
        continue;
    
    return spi_receivedata(SPIx);
}
