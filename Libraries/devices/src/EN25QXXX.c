//���������ϵ�EN25Q�洢оƬ
//���ȣ���ʵ��������Ĺ��ܣ���ȡоƬ��id����ȡоƬ������
//Ȼ��ʵ�ֲ�����дһ���ֽڵĲ�����
//���ʵ��д�����ⳤ�ȵ�����
//ʹ��һ��GPIO�ڣ������GPIO��������EN25Q��chip select
//ʹ��spi1
//��ȡEN25Q��id��

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
	//���жϴ���ĵ�ַ�����Ƿ����Ҫ��
	//����д����
	//addr + bufnum�Ľ�����ܳ���EN25Q�����һ����ֵַ
    //�������ַд�벻����EN25Qʣ��ռ䳤�ȵ�����
	//���ж�Ŀ���ַ��������ַ��������ƫ����������ʣ��ռ䡣
	//�ж�bufnum�ĳ��Ⱥ�����ʣ��ռ�ĳ���
	//while
	//���������������ݣ�����secbuf��
	//�ж����������ݣ���0xFF�Ƚϣ��������Ƿ���в�������
	//д���������ж�bufnum��ʵ��д�������
	//���Ŀ������д���£��ͰѴ��ݵĲ�������ã����µ���д�뺯����������Ĳ�������һ��
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
    //���д��ĵ�ַ����ĳһ��ҳ�ڵ�ƫ�Ƶ�ַ
    //�Ƚ�ҳ��ʣ��ռ��ʵ��д����ֽ������������ݵĲ���
    //���ʵ��д����ֽ�����һ��ҳ��д���꣬���ظ�����д�뺯����ǰ���ǽ�ÿ�δ��ݵĲ��������
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
    

