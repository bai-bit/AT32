//���������ϵ�EN25Q�洢оƬ
//���ȣ���ʵ��������Ĺ��ܣ���ȡоƬ��id����ȡоƬ������
//Ȼ��ʵ�ֲ�����дһ���ֽڵĲ�����
//���ʵ��д�����ⳤ�ȵ�����
//ʹ��һ��GPIO�ڣ������GPIO��������EN25Q��chip select
//ʹ��spi1
//��ȡEN25Q��id��
//����������spi_flash�Ĳ����߼���������Ӧ�Ĳ�����ʵ�ֶ�EN25Q��programer

#include<EN25QXXX.h>


//initialization EN25Q
uint16_t EN25QXXX_init(SPI_Type *SPIx)
{
	uint16_t EN25Q_ID = 0;
   	
	GPIO_Init(HW_GPIOA,gpio_pin_4, gpio_speed_50MHz, mode_out_pp);
    GPIO_PinWrite(HW_GPIOA, gpio_pin_4, 1);
    
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
	spi_resetbaud(SPI1,SPI_BAUDRATE_64);
	EN25QXXX_active_mode(SPIx);
	EN25Q_ID |= EN25QXXX_readID(SPI1);
	    
	return EN25Q_ID;
}


uint16_t EN25QXXX_readID(SPI_Type *SPIx)
{
    //chip select low
    //send read instruction
    //send 00h thrice
    //then receive data by send ffh
    uint16_t ret = 0;

    GPIO_PinWrite(HW_GPIOA, gpio_pin_4,0);
    spi_RWdata(SPIx,READ_DEVICE_ID);
	spi_RWdata(SPIx,0x00);
	spi_RWdata(SPIx,0x00);
	spi_RWdata(SPIx,0x00);
	ret |= spi_RWdata(SPIx,SEND_DEFAULT_VALUE) << 8;
	ret |= spi_RWdata(SPIx,SEND_DEFAULT_VALUE);
    GPIO_PinToggle(HW_GPIOA, gpio_pin_4);
	
    return ret;
}


//read data for en25q
uint32_t EN25QXXX_read_data(SPI_Type *SPIx,uint32_t addr,u8 *rbuf,uint32_t num)
{
	//��ID,��status,��data
   	uint32_t i = 0;
	
    GPIO_PinWrite(HW_GPIOA,gpio_pin_4,0);
	
	spi_RWdata(SPIx,EN25Q_FAST_READ);
	spi_RWdata(SPIx,(u8)((addr) >> 16));
	spi_RWdata(SPIx,(u8)((addr) >> 8));
	spi_RWdata(SPIx,(u8)(addr));
	
	for(i = 0;i < num;i++)
		rbuf[i] = spi_RWdata(SPIx,SEND_DEFAULT_VALUE);
	
	GPIO_PinWrite(HW_GPIOA,gpio_pin_4,1);
    return sizeof(rbuf);
}
//read status register for en25q
uint8_t EN25QXXX_read_register(SPI_Type *SPIx)
{
    uint8_t status = 0;
		
	GPIO_PinWrite(HW_GPIOA,gpio_pin_4,0);
	
	spi_RWdata(SPIx,EN25Q_READ_STATUS);
	status = spi_RWdata(SPIx,SEND_DEFAULT_VALUE);

    GPIO_PinToggle(HW_GPIOA,gpio_pin_4);
    
    return status;
}
//write enable
void EN25QXXX_write_enable(SPI_Type *SPIx)
{
    GPIO_PinWrite(HW_GPIOA,gpio_pin_4,0);
    spi_RWdata(SPIx,EN25Q_WRITE_ENABLE);
    GPIO_PinToggle(HW_GPIOA,gpio_pin_4);    
}
//write disable
void EN25QXXX_write_disable(SPI_Type *SPIx)
{
    GPIO_PinWrite(HW_GPIOA,gpio_pin_4,0);
    spi_RWdata(SPIx,EN25Q_WRITE_DISABLE);
    GPIO_PinToggle(HW_GPIOA,gpio_pin_4);    
}
//write data to en25q
u8 secbuf[SECSIZE];
uint32_t EN25QXXX_write_data(SPI_Type *SPIx,uint32_t addr,uint8_t *buf,uint32_t bufnum)
{
	
	uint32_t sectorid = 0,secoffset = 0,secsurplus = 0;
	uint32_t i = 0,m = 0;
	uint8_t *temp_buf = secbuf;
	
	if(addr < EN25Q_BASE && addr > SECSIZE * SECNUM)
		return 0;
	if(bufnum > SECSIZE * SECNUM - addr)
		bufnum = SECSIZE * SECNUM - addr;
	sectorid = addr / SECSIZE;
	secoffset = addr % SECSIZE;
	secsurplus = SECSIZE - secoffset;
	if(secsurplus > bufnum)
		secsurplus = bufnum;
	m = secsurplus;

	while(1)
	{
		EN25QXXX_read_data(SPIx,sectorid * SECSIZE,temp_buf,SECSIZE);
		for(i = 0;i < secsurplus;i++)
			if(temp_buf[secoffset + i] != 0xFF)
				break;

		if(i < secsurplus)
		{
			EN25QXXX_erase_sector(SPIx,sectorid * SECSIZE);
			
			for(i = 0;i < secsurplus;i++)
				temp_buf[secoffset + i] = buf[i];
			
			EN25QXXX_write_nocheck(SPIx,sectorid * SECSIZE,temp_buf,SECSIZE);
		}
		else
			EN25QXXX_write_nocheck(SPIx,addr,buf,bufnum);
			
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
			m += secsurplus;
		}
	}
	return m;
}

//256 or >256
void EN25QXXX_write_page(SPI_Type *SPIx,const uint32_t addr,u8 *buf,uint16_t bufnum)
{
    uint16_t i;
	
    EN25QXXX_write_enable(SPIx);
    EN25QXXX_wait_busy(SPIx);

    GPIO_PinWrite(HW_GPIOA,gpio_pin_4,0);
    spi_RWdata(SPIx,EN25Q_PAGE_PROGRAM);
    spi_RWdata(SPIx,(u8)((addr) >> 16));
    spi_RWdata(SPIx,(u8)((addr) >> 8));
    spi_RWdata(SPIx,(u8)(addr));

    for(i = 0;i < bufnum;i++)
		spi_RWdata(SPIx,buf[i]);
	
    GPIO_PinToggle(HW_GPIOA,gpio_pin_4);
    EN25QXXX_wait_busy(SPIx);
}

void EN25QXXX_write_nocheck(SPI_Type *SPIx,uint32_t addr,u8 *buf,uint32_t bufnum)
{
    //���д��ĵ�ַ����ĳһ��ҳ�ڵ�ƫ�Ƶ�ַ
    //�Ƚ�ҳ��ʣ��ռ��ʵ��д����ֽ������������ݵĲ���
    //���ʵ��д����ֽ�����һ��ҳ��д���꣬���ظ�����д�뺯����ǰ���ǽ�ÿ�δ��ݵĲ��������
    uint32_t pagesurplus = 0;
   
    pagesurplus = PAGESIZE - ((addr) % PAGESIZE);
	if(pagesurplus >= bufnum)
		pagesurplus = bufnum;
	EN25QXXX_write_page(SPIx,(addr),buf,pagesurplus);
	while(pagesurplus < bufnum)
	{
		buf = buf + pagesurplus;
		addr = (addr) + pagesurplus;
		bufnum = bufnum - pagesurplus;
		if(bufnum >= PAGESIZE)
			pagesurplus = PAGESIZE;
		else
			pagesurplus = bufnum;
		
		EN25QXXX_write_page(SPIx,(addr),buf,pagesurplus);
	}
}

void EN25QXXX_erase_sector(SPI_Type *SPIx,const uint32_t addr)
{
    uint32_t temp = addr;
	
    EN25QXXX_write_enable(SPIx);
    EN25QXXX_wait_busy(SPIx);
    
    GPIO_PinWrite(HW_GPIOA,gpio_pin_4,0);
		
    spi_RWdata(SPIx,EN25Q_SECTOR_ERASE);
    spi_RWdata(SPIx,(u8)((temp) >> 16));
    spi_RWdata(SPIx,(u8)((temp) >> 8));
    spi_RWdata(SPIx,(u8)(temp));

    GPIO_PinToggle(HW_GPIOA,gpio_pin_4);  
    
	EN25QXXX_wait_busy(SPIx);
}
//chip erase
void EN25QXXX_chip_erase(SPI_Type *SPIx)
{
    EN25QXXX_write_enable(SPIx);
    EN25QXXX_wait_busy(SPIx);
    
    GPIO_PinWrite(HW_GPIOA,gpio_pin_4,0);
    spi_RWdata(SPIx,EN25Q_CHIP_ERASE);
    GPIO_PinToggle(HW_GPIOA,gpio_pin_4);
    EN25QXXX_wait_busy(SPIx);
}
//wait busy
void EN25QXXX_wait_busy(SPI_Type *SPIx)
{
    while((EN25QXXX_read_register(SPIx) & 0x01) == 0x01) 
        continue;    
}

//active mode
void EN25QXXX_active_mode(SPI_Type *SPIx)
{
    GPIO_PinWrite(HW_GPIOA,gpio_pin_4,0);
	spi_RWdata(SPIx,EN25Q_RELEASE_SLEEP);
    GPIO_PinWrite(HW_GPIOA,gpio_pin_4,1);
    delayms(10);
}

//sleep mode
void EN25QXXX_sleep_mode(SPI_Type *SPIx)
{
    GPIO_PinWrite(HW_GPIOA,gpio_pin_4,0);
	spi_RWdata(SPIx,EN25Q_DEEP_POWER);
	GPIO_PinWrite(HW_GPIOA,gpio_pin_4,1);
	delayms(10);
}

//������ļ��У�ʵ�ֶ�flash�Ĳ���
//ʹ��read��write��clear��close���ĸ�����ʵ�ֶ��ֹ���
//open����flash,ȷ��ʹ��spiͨ����
//read��ʵ�ֶ�flash�Ķ�ȡ���ݣ���ȡid����ȡָ����ַ�����ݣ���ȡstatus
//write��ʵ�ֶ�flash��д�������д��״̬����ָ����ַд������
//clear��ʵ�ֲ���������sector��chip_erase
//close���ر�flash

    
uint32_t EN25QXXX_read(SPI_Type *SPIx,uint32_t addr,uint8_t *rbuf,uint32_t num,uint8_t cmd)
{
	uint32_t ret;
	
	if(cmd == ID)
	{
		ret = EN25QXXX_readID(SPIx);
		rbuf[0] = (uint8_t)(ret >> 8);
		rbuf[1] = (uint8_t)(ret);
	}
	else if(cmd == STATUS)
		ret = EN25QXXX_read_register(SPIx);
	else if(cmd == DATA)
		ret = EN25QXXX_read_data(SPIx,addr,rbuf,num);
	return ret;
}

uint32_t EN25QXXX_write(SPI_Type *SPIx,uint32_t addr,uint8_t *buf,uint32_t num,uint8_t cmd)
{
	uint32_t i = 0;
	
	if(cmd == DATA)
		i = EN25QXXX_write_data(SPIx,addr,buf,num);
//	else if(cmd == STATUS)
//		EN25QXXX_write_register(SPIx,buf);
	return i;

}

void EN25QXXX_clear(SPI_Type *SPIx,uint32_t addr,uint8_t cmd)
{
	if(cmd == SECTOR)
		EN25QXXX_erase_sector(SPIx,addr);
	else if(cmd == CHIP)
		EN25QXXX_chip_erase(SPIx);

}

void EN25QXXX_close(SPI_Type *SPIx)
{
	EN25QXXX_sleep_mode(SPIx);
	spi_cmd(SPIx,DISABLE);
}
