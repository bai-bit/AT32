#include "SPI.h"

#define SPI_CPOLY_RESET         (0x0007)
#define SPI_CR1_Mask            (0xE9F3)
#define SPI_MODE_SEL            (0xF7FF)

SPI_Type *SPI_list[] = {SPI1,SPI2,SPI3,SPI4};

void SPI_Init(uint8_t SPIx, uint32_t baud)
{
    //首先配置为缺省值。全是0
	//然后根据自己的需要配寄存器
	//配置的内容：数据的传输方向，全双工，8位数据帧，lsb的格式帧，分频系数256，主机模式，cpol = 0，cpha = 1，nss值，crc
	//使能SPI的时钟
	//最后使能SPI
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
    
    SPI_Cmd(SPIx, ENABLE);
}
void SPI_SetMode(uint8_t SPIx, uint8_t mode)
{
    //MSTEN
    while(SPI_GetFlagStatus(SPIx, SPI_STS_BSY) == SET)
        continue;
    SPI_Cmd(SPIx,DISABLE);
    SPI_list[SPIx]->CTRL1 &= ~(SPI_CTRL1_MSTEN);
    SPI_list[SPIx]->CTRL1 |= mode;
    SPI_Cmd(SPIx,ENABLE);
}
void SPI_SetTR(uint8_t SPIx, uint8_t mode)
{
    while(SPI_GetFlagStatus(SPIx, SPI_STS_BSY) == SET)
        continue;
    SPI_Cmd(SPIx,DISABLE);
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
 
    SPI_Cmd(SPIx,ENABLE);
}
void SPI_SetTransMode(uint8_t SPIx, uint8_t transmode)
{
    while(SPI_GetFlagStatus(SPIx, SPI_STS_BSY) == SET)
        continue;
    SPI_Cmd(SPIx,DISABLE);
    SPI_list[SPIx]->CTRL1 &=~(SPI_CTRL1_LSBEN);
    SPI_list[SPIx]->CTRL1 |= transmode;
    SPI_Cmd(SPIx,ENABLE);
}
void SPI_resetFirstBits(uint8_t SPIx, uint8_t first_bits)
{
    while(SPI_GetFlagStatus(SPIx, SPI_STS_BSY) == SET)
        continue;
    SPI_Cmd(SPIx,DISABLE);
    SPI_list[SPIx]->CTRL1 &= ~(SPI_CTRL1_DFF16);
    SPI_list[SPIx]->CTRL1 |= first_bits;
    SPI_Cmd(SPIx,ENABLE);
}
void SPI_Cmd(uint8_t SPIx, FunctionalState status)
{
    if(status == ENABLE)
        SPI_list[SPIx]->CTRL1 |= SPI_CTRL1_SPIEN;
    else
        SPI_list[SPIx]->CTRL1 &= ~SPI_CTRL1_SPIEN;
}

FlagStatus SPI_GetFlagStatus(uint8_t SPIx, uint16_t SPI_flag)
{
	if(SPI_list[SPIx]->STS & SPI_flag)
		return SET;
	else
		return RESET;
}

//add baud reset functional achieve
void SPI_SetBaud(uint8_t SPIx, uint32_t baud)
{
    while(SPI_GetFlagStatus(SPIx, SPI_STS_BSY) == SET)
        continue;
    SPI_Cmd(SPIx,DISABLE);
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
    SPI_Cmd(SPIx,ENABLE);
}
//add operation SPI->DT register function
void SPI_SendData(uint8_t SPIx, uint8_t data)
{
    SPI_list[SPIx]->DT = data;
}
uint8_t SPI_ReceiveData(uint8_t SPIx)
{
    return SPI_list[SPIx]->DT;
}
//add SPI read_write functional achieve
uint8_t SPI_TransferData(uint8_t SPIx, uint8_t data)
{
    uint32_t time = 0x10000;
    
    while(SPI_GetFlagStatus(SPIx, SPI_STS_BSY) == SET && time--)
        continue;
    time = 0x10000;
    SPI_SendData(SPIx, data);
    while(SPI_GetFlagStatus(SPIx, SPI_STS_BSY) == SET && time--)
        continue;
    
    return SPI_ReceiveData(SPIx);
}


