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
	uint16_t EN25Q_ID = 0;
    spim_init(SPIx); 
    spi_resetbaud(SPI1,SPI_BAUDRATE_64);
	EN25Q_ID |= EN25QXXX_readID(SPI1);
	printf("EN25Q_ID = [%X]\t\n",EN25Q_ID);
}

void spim_init(SPI_Type *SPIx)
{
    //initialization GPIO   chip select
    GPIO_Init(HW_GPIOA,gpio_pin_4, gpio_speed_50MHz, mode_out_pp);
    GPIO_PinWrite(HW_GPIOA, gpio_pin_4, 1);
    
    
	//initialization SPI1 interface
    SPI_InitType SPI_InitStruct;
			
	SPI_InitStruct.SPI_Transmode = SPI_FULLDUPLEX;
	SPI_InitStruct.SPI_Mode = SPI_CTRL1_MSTEN | SPI_CTRL1_ISS;
	SPI_InitStruct.SPI_FrameSize = SPI_DATASIZE_8Bits;
	SPI_InitStruct.SPI_CPHA = SPI_CTRL1_CPHA;
	SPI_InitStruct.SPI_CPOL = SPI_CTRL1_CPOL;
	SPI_InitStruct.SPI_FirstBit = SPI_FRISTBIT_MSB;
	SPI_InitStruct.SPI_NSSSET = SPI_CTRL1_SWNSSEN;
	SPI_InitStruct.SPI_MCL = SPI_CTRL1_MCLKP;
	SPI_InitStruct.SPI_CPOLY = SPI_CPOLY_RESET;

	spi_init(SPIx,&SPI_InitStruct);
}



uint16_t EN25QXXX_readID(SPI_Type *SPIx)
{
    //chip select low
    //send read instruction
    //send 00h thrice
    //then receive data by send ffh
    uint16_t ret = 0;
    GPIO_PinWrite(HW_GPIOA, gpio_pin_4,0);
    
    spi_RWdata(SPIx, READ_DEVICE_ID);
    spi_RWdata(SPIx, 0X00);
    spi_RWdata(SPIx, 0X00);
    spi_RWdata(SPIx, 0X00);
    
    ret |= (spi_RWdata(SPIx, SEND_DEFAULT_VALUE) << 8);
    ret |= spi_RWdata(SPIx, SEND_DEFAULT_VALUE);
  
    GPIO_PinToggle(HW_GPIOA, gpio_pin_4);
    return ret;
}

//read data for en25q
u8 *EN25QXXX_read_data(uint32_t addr,u8 *buf,uint32_t num)
{
   	uint32_t i = 0;
    GPIO_PinWrite(HW_GPIOA,gpio_pin_4,0);
	spi_RWdata(SPI1,EN25Q_FAST_READ);
	spi_RWdata(SPI1,(u8)(addr >> 16));
	spi_RWdata(SPI1,(u8)(addr >> 8));
	spi_RWdata(SPI1,(u8)addr);
	//spi_RWdata(SPI1,SEND_DEFAULT_VALUE);
	for(i = 0;i < num ;i++)
	{
		buf[i] = spi_RWdata(SPI1,SEND_DEFAULT_VALUE);
	}
	GPIO_PinWrite(HW_GPIOA,gpio_pin_4,1);
    return buf;
}
//read status register for en25q
u8 EN25QXXX_read_register(void)
{
    u8 status = 0;
	GPIO_PinWrite(HW_GPIOA,gpio_pin_4,0);
	spi_RWdata(SPI1,EN25Q_READ_STATUS);
	spi_RWdata(SPI1,SEND_DEFAULT_VALUE);
    status = spi_RWdata(SPI1,SEND_DEFAULT_VALUE);
	GPIO_PinToggle(HW_GPIOA,gpio_pin_4);
    
    return status;
}
//write enable
void EN25QXXX_write_enable(void)
{
    GPIO_PinWrite(HW_GPIOA,gpio_pin_4,0);
    spi_RWdata(SPI1,EN25Q_WRITE_ENABLE);
    GPIO_PinToggle(HW_GPIOA,gpio_pin_4);    
}
//write disable
void EN25QXXX_write_disable(void)
{
    GPIO_PinWrite(HW_GPIOA,gpio_pin_4,0);
    spi_RWdata(SPI1,EN25Q_WRITE_DISABLE);
    GPIO_PinToggle(HW_GPIOA,gpio_pin_4);    
}
//write data to en25q
void EN25QXXX_write_data(uint32_t addr,u8 *buf,uint32_t bufnum)
{
	//先判断传入的地址参数是否符合要求
	//处理写保护
	//addr + bufnum的结果不能超过EN25Q的最后一个地址值
    //在任意地址写入不超过EN25Q剩余空间长度的数据
	//先判断目标地址，扇区地址，扇区内偏移量，扇区剩余空间。
	//判断bufnum的长度和扇区剩余空间的长度
	//while
	//读出该扇区的内容，放在secbuf中
	//判断扇区的内容，和0xFF比较，来决定是否进行擦除操作
	//写入扇区后，判断bufnum和实际写入的数量
	//如果目标扇区写不下，就把传递的参数处理好，重新调用写入函数，将上面的步骤再来一遍
	uint32_t sectorid,secoffset,secsurplus;
	u8 secbuf[SECSIZE] = "";
	uint32_t i = 0;
	
	if(addr < EN25Q_BASE && addr > SECSIZE * SECNUM)
		return ;
	if(bufnum > SECSIZE * SECNUM - addr)
		bufnum = SECSIZE * SECNUM - addr;
	sectorid = addr / SECSIZE;
	secoffset = addr % SECSIZE;
	secsurplus =SECSIZE - secoffset;
	if(secsurplus > bufnum)
		secsurplus = bufnum;
	while(1)
	{
		EN25QXXX_read_data(sectorid * SECSIZE,secbuf,SECSIZE);
		for(i = 0;i < secsurplus;i++)
			if(secbuf[secsurplus + i] != 0xFF)
				break;
		if(i < secsurplus)
		{
			EN25QXXX_erase_sector(sectorid);
			for(i = 0;i < secsurplus;i++)
				secbuf[secsurplus + i] = buf[i];
			EN25QXXX_write_nocheck(sectorid * SECSIZE,secbuf,SECSIZE);
		}
		else
			EN25QXXX_write_nocheck(sectorid * SECSIZE,buf,bufnum);
		if(secsurplus == bufnum)
			break;
		else
		{
			bufnum -= secsurplus;
			addr += secsurplus;
			buf += secsurplus;
			sectorid++;
			secoffset = 0;
			if(bufnum > SECSIZE)
				secsurplus = SECSIZE;
			else
				secsurplus = bufnum;
		}
		
	}
	
    
}
//256 or >256
void EN25QXXX_write_page(uint32_t addr,u8 *buf,u8 bufnum)
{
    u8 i;
    EN25QXXX_write_enable();
    EN25QXXX_wait_busy();
    
    GPIO_PinWrite(HW_GPIOA,gpio_pin_4,0);
    spi_RWdata(SPI1,EN25Q_PAGE_PROGRAM);
    spi_RWdata(SPI1,(u8)(addr >> 16));
    spi_RWdata(SPI1,(u8)(addr >> 8));
    spi_RWdata(SPI1,(u8)addr);
    for(i = 0;i < bufnum;i++)
        spi_RWdata(SPI1,buf[i]);
    GPIO_PinToggle(HW_GPIOA,gpio_pin_4);
    EN25QXXX_wait_busy();
}

void EN25QXXX_write_nocheck(uint32_t addr,u8 *buf,uint32_t bufnum)
{
    //检查写入的地址，在某一个页内的偏移地址
    //比较页内剩余空间和实际写入的字节数，决定传递的参数
    //如果实际写入的字节数在一个页内写不完，就重复调用写入函数，前提是将每次传递的参数处理好
    uint32_t pagesurplus = 0;
    
    pagesurplus = PAGESIZE - addr % PAGESIZE;
	if(pagesurplus > bufnum)
		pagesurplus = bufnum;
	EN25QXXX_write_page(addr,buf,pagesurplus);
	while(pagesurplus < bufnum)
	{
		buf = buf + pagesurplus;
		addr = addr + pagesurplus;
		bufnum = bufnum - pagesurplus;
		if(bufnum > PAGESIZE)
			pagesurplus = PAGESIZE;
		else
			pagesurplus = bufnum;
		EN25QXXX_write_page(addr,buf,pagesurplus);
	}
    
}

//erase sector
void EN25QXXX_erase_sector(uint32_t addr)
{
    uint32_t secaddr = 0;
	u8 status1 = 0;
	
    secaddr = addr * SECSIZE;
    EN25QXXX_write_enable();
    EN25QXXX_wait_busy();
	
    
    GPIO_PinWrite(HW_GPIOA,gpio_pin_4,0);
    spi_RWdata(SPI1,EN25Q_SECTOR_ERASE);
    spi_RWdata(SPI1,(u8)(secaddr >> 16));
    spi_RWdata(SPI1,(u8)(secaddr >> 8));
    spi_RWdata(SPI1,(u8)secaddr);
    GPIO_PinToggle(HW_GPIOA,gpio_pin_4);  
    
	status1 = EN25QXXX_read_register();
	printf("status1 = [%X]\t\n",status1);
    EN25QXXX_wait_busy();
	status1 = EN25QXXX_read_register();
	printf("status1 = [%X]\t\n",status1);
}
//chip erase
void EN25QXXX_chip_erase(void)
{
    EN25QXXX_write_enable();
    EN25QXXX_wait_busy();
    
    GPIO_PinWrite(HW_GPIOA,gpio_pin_4,0);
    spi_RWdata(SPI1,EN25Q_CHIP_ERASE);
    GPIO_PinToggle(HW_GPIOA,gpio_pin_4);
    EN25QXXX_wait_busy();
}
//wait busy
void EN25QXXX_wait_busy(void)
{
    while((EN25QXXX_read_register() & 0x01) == 0x01) 
        continue;    
}

//active mode
void EN25QXXX_active_mode(void)
{
    GPIO_PinWrite(HW_GPIOA,gpio_pin_4,0);
    spi_RWdata(SPI1,0xAB);
    GPIO_PinWrite(HW_GPIOA,gpio_pin_4,1);
    delayms(10);
}

//sleep mode
void EN25QXXX_sleep_mode(void)
{
    GPIO_PinWrite(HW_GPIOA,gpio_pin_4,0);
	spi_RWdata(SPI1,0xB9);
	GPIO_PinWrite(HW_GPIOA,gpio_pin_4,1);
	delayms(10);
}
    

