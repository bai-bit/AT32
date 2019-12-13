#include<spi.h>

void spi_init(SPI_Type *SPIx,SPI_InitType *SPI_InitStruct)
{
    //首先配置为缺省值。全是0
	//然后根据自己的需要配寄存器
	//配置的内容：数据的传输方向，全双工，8位数据帧，lsb的格式帧，分频系数256，主机模式，cpol = 0，cpha = 1，nss值，crc
	//使能spi的时钟
	//最后使能spi
	if(SPIx == SPI1)
		RCC->APB2EN |= RCC_APB2EN_SPI1EN;
	else if(SPIx == SPI2)
		RCC->APB1EN |= RCC_APB1EN_SPI2EN;
	else if(SPIx == SPI3)
		RCC->APB1EN |= RCC_APB1EN_SPI3EN;
	else if(SPIx == SPI4)
		RCC->APB1EN |= RCC_APB1EN_SPI4EN;
	if(SPIx == SPI1)
	{
		GPIO_Init(HW_GPIOA, gpio_pin_5, gpio_speed_50MHz, mode_af_pp);
		GPIO_Init(HW_GPIOA, gpio_pin_7, gpio_speed_50MHz, mode_af_pp);
		GPIO_Init(HW_GPIOA, gpio_pin_6, gpio_speed_inno,mode_in_floating);
	}
	
	SPIx->CTRL2 &= ~SPI_CTRL2_MCLKP_3;
	SPIx->CTRL1 &= SPI_CR1_Mask;
	
	SPIx->CTRL1 |= (SPI_InitStruct->SPI_Transmode | SPI_InitStruct->SPI_Mode | SPI_InitStruct->SPI_FrameSize | SPI_InitStruct->SPI_CPOL | SPI_InitStruct->SPI_CPHA | SPI_InitStruct->SPI_NSSSET | SPI_InitStruct->SPI_MCL);
    SPIx->CTRL1 &=  SPI_FristBit_Mask;
	SPIx->CTRL1 |= SPI_InitStruct->SPI_FirstBit;

    SPIx->I2SCTRL &= SPI_MODE_SEL;
    SPIx->CPOLY = SPI_InitStruct->SPI_CPOLY;

    spi_cmd(SPIx, ENABLE);
}

void spi_cmd(SPI_Type *SPIx,FunctionalState status)
{
    if(status == ENABLE)
        SPIx->CTRL1 |= SPI_CTRL1_SPIEN;
    else
        SPIx->CTRL1 &= ~SPI_CTRL1_SPIEN;
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
    
    while(spi_GetFlagStatus(SPIx,SPI_Flag_BSY) == SET && time--)
        continue;
    time = 0x10000;
    spi_senddata(SPIx,data);
    while(spi_GetFlagStatus(SPIx,SPI_Flag_BSY) == SET && time--)
        continue;
    
    return spi_receivedata(SPIx);
}
