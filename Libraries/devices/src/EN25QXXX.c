//操作板子上的EN25Q存储芯片
//首先，先实现最基本的功能：读取芯片的id，读取芯片的内容
//然后，实现擦除和写一个字节的操作。
//最后，实现写入任意长度的数据
//使能一个GPIO口，用这个GPIO口来控制EN25Q的chip select
//使能spi1
//读取EN25Q的id号

#include<EN25QXXX.h>


//initialization EN25Q
void EN25QXXX_init(SPI_Type *SPIx)
{
    spim_init(SPIx); 
    spi_resetbaud(SPI1,SPI_BAUDRATE_64);
}

void spim_init(SPI_Type *SPIx)
{
    //使能复用功能时钟，GPIO口的时钟 SCK PB1,CS PA8,MOSI PA11,MISO PA12,
    //初始化GPIO口
    RCC->APB2EN |= RCC_APB2EN_AFIOEN;
    GPIO_Init(HW_GPIOA,gpio_pin_4, gpio_speed_50MHz, mode_out_pp);
    GPIO_PinWrite(HW_GPIOA, gpio_pin_4, 1);
    
    //使能外部spi flash接口
//    AFIO->MAP2 |= AFIO_MAP2_EXT_FLASH_REMAP;
    //初始化SPI接口
    spi1_init();
}

uint16_t EN25QXXX_readID(SPI_Type *SPIx)
{
    //chip select low
    //send read instruction
    //send 00h thrice
    //then receive data by send ffh
    uint16_t ret = 0;
    GPIO_PinWrite(HW_GPIOA, gpio_pin_4,0);
    
    spi_RWdata(SPIx, 0X90);
    spi_RWdata(SPIx, 0X00);
    spi_RWdata(SPIx, 0X00);
    spi_RWdata(SPIx, 0X00);
    
    ret |= (spi_RWdata(SPIx, 0xFF) << 8);
    ret |= spi_RWdata(SPIx, 0xFF);
  
    GPIO_PinToggle(HW_GPIOA, gpio_pin_4);
    return ret;
}

//read data for en25q
u8 EN25QXXX_read_data(void)
{
    u8 data = 0;
    
    return data;
}
//read status register for en25q
u8 EN25QXXX_read_register(void)
{
    u8 status = 0;
    
    return status;
}
//write enable
void EN25QXXX_write_enable(void)
{
    
    
}
//write disable
void EN25QXXX_write_disable(void)
{
    
    
}
//write data to en25q
void EN25QXXX_write_data(u8 buf,uint32_t addr,uint32_t bufnum)
{
    
    
}
//256 or >256
void EN25QXXX_write_page(u8 buf,uint32_t addr,uint32_t bufnum)
{
    
    
}
void EN25QXXX_write_nocheck(u8 buf,uint32_t addr,uint32_t bufnum)
{
    
    
}
//get status flag at present for en25q
u8 EN25QXXX_get_status(void)
{
    
    return 0;
}
//erase sector
void EN25QXXX_erase_sector(uint32_t addr)
{
    
    
}
//wait busy
void EN25QXXX_wait_busy(void)
{
    
    
}

//active mode
void EN25QXXX_active_mode(void)
{
    GPIO_PinWrite(HW_GPIOA,gpio_pin_4,0);
    spi_RWdata(SPI1,0xAB);
    GPIO_PinWrite(HW_GPIOA,gpio_pin_4,1);
    delayms(100);
}

//sleep mode
void EN25QXXX_sleep_mode(void)
{
    
    
}
