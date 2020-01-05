#include<EN25QXXX.h>

EN25Q_dev_t EN25Q_operation;

//initialization EN25Q
void EN25Q_module_init(EN25Q_dev_t *opers)
{
    memcpy(&EN25Q_operation,opers,sizeof(EN25Q_dev_t)); 
}
uint16_t EN25QXXX_init(SPI_Type *SPIx)
{
    uint16_t EN25Q_ID = 0;
    
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

    EN25Q_operation.cs(EN25Q_operation.GPIOx, EN25Q_operation.Pin,0);
    EN25Q_operation.msg_que(SPIx,READ_DEVICE_ID);
    EN25Q_operation.msg_que(SPIx,0x00);
    EN25Q_operation.msg_que(SPIx,0x00);
    EN25Q_operation.msg_que(SPIx,0x00);
    ret |= EN25Q_operation.msg_que(SPIx,SEND_DEFAULT_VALUE) << 8;
    ret |= EN25Q_operation.msg_que(SPIx,SEND_DEFAULT_VALUE);
    EN25Q_operation.cs(EN25Q_operation.GPIOx, EN25Q_operation.Pin,1);
    
    return ret;
}


//read data for en25q
uint32_t EN25QXXX_read_data(SPI_Type *SPIx,uint32_t addr,u8 *rbuf,uint32_t num)
{
    //读ID,读status,读data
    uint32_t i = 0;
    
    EN25Q_operation.cs(EN25Q_operation.GPIOx,EN25Q_operation.Pin,0);
	
    EN25Q_operation.msg_que(SPIx,EN25Q_FAST_READ);
    EN25Q_operation.msg_que(SPIx,(u8)((addr) >> 16));
    EN25Q_operation.msg_que(SPIx,(u8)((addr) >> 8));
    EN25Q_operation.msg_que(SPIx,(u8)(addr));
    
    for(i = 0;i < num;i++)
        rbuf[i] = EN25Q_operation.msg_que(SPIx,SEND_DEFAULT_VALUE);
    
    EN25Q_operation.cs(EN25Q_operation.GPIOx,EN25Q_operation.Pin,1);
    return sizeof(rbuf);
}
//read status register for en25q
uint8_t EN25QXXX_read_register(SPI_Type *SPIx)
{
    uint8_t status = 0;
    
    EN25Q_operation.cs(EN25Q_operation.GPIOx,EN25Q_operation.Pin,0);
    
    EN25Q_operation.msg_que(SPIx,EN25Q_READ_STATUS);
    status = EN25Q_operation.msg_que(SPIx,SEND_DEFAULT_VALUE);
    
    EN25Q_operation.cs(EN25Q_operation.GPIOx,EN25Q_operation.Pin,1);
    
    return status;
}
//write enable
void EN25QXXX_write_enable(SPI_Type *SPIx)
{
    EN25Q_operation.cs(EN25Q_operation.GPIOx,EN25Q_operation.Pin,0);
    EN25Q_operation.msg_que(SPIx,EN25Q_WRITE_ENABLE);
    EN25Q_operation.cs(EN25Q_operation.GPIOx,EN25Q_operation.Pin,1);    
}
//write disable
void EN25QXXX_write_disable(SPI_Type *SPIx)
{
    EN25Q_operation.cs(EN25Q_operation.GPIOx,EN25Q_operation.Pin,0);
    EN25Q_operation.msg_que(SPIx,EN25Q_WRITE_DISABLE);
    EN25Q_operation.cs(EN25Q_operation.GPIOx,EN25Q_operation.Pin,1);    
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
    
    EN25Q_operation.cs(EN25Q_operation.GPIOx,EN25Q_operation.Pin,0);
    EN25Q_operation.msg_que(SPIx,EN25Q_PAGE_PROGRAM);
    EN25Q_operation.msg_que(SPIx,(u8)((addr) >> 16));
    EN25Q_operation.msg_que(SPIx,(u8)((addr) >> 8));
    EN25Q_operation.msg_que(SPIx,(u8)(addr));
    
    for(i = 0;i < bufnum;i++)
        EN25Q_operation.msg_que(SPIx,buf[i]);
    
    EN25Q_operation.cs(EN25Q_operation.GPIOx,EN25Q_operation.Pin,1);
    EN25QXXX_wait_busy(SPIx);
}

void EN25QXXX_write_nocheck(SPI_Type *SPIx,uint32_t addr,u8 *buf,uint32_t bufnum)
{
    //检查写入的地址，在某一个页内的偏移地址
    //比较页内剩余空间和实际写入的字节数，决定传递的参数
    //如果实际写入的字节数在一个页内写不完，就重复调用写入函数，前提是将每次传递的参数处理好
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
    
    EN25Q_operation.cs(EN25Q_operation.GPIOx,EN25Q_operation.Pin,0);
    
    EN25Q_operation.msg_que(SPIx,EN25Q_SECTOR_ERASE);
    EN25Q_operation.msg_que(SPIx,(u8)((temp) >> 16));
    EN25Q_operation.msg_que(SPIx,(u8)((temp) >> 8));
    EN25Q_operation.msg_que(SPIx,(u8)(temp));
    
    EN25Q_operation.cs(EN25Q_operation.GPIOx,EN25Q_operation.Pin,1);  
    
    EN25QXXX_wait_busy(SPIx);
}
//chip erase
void EN25QXXX_chip_erase(SPI_Type *SPIx)
{
    EN25QXXX_write_enable(SPIx);
    EN25QXXX_wait_busy(SPIx);
    
    EN25Q_operation.cs(EN25Q_operation.GPIOx,EN25Q_operation.Pin,0);
    EN25Q_operation.msg_que(SPIx,EN25Q_CHIP_ERASE);
    EN25Q_operation.cs(EN25Q_operation.GPIOx,EN25Q_operation.Pin,1);
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
    EN25Q_operation.cs(EN25Q_operation.GPIOx,EN25Q_operation.Pin,0);
    EN25Q_operation.msg_que(SPIx,EN25Q_RELEASE_SLEEP);
    EN25Q_operation.cs(EN25Q_operation.GPIOx,EN25Q_operation.Pin,1);
}

//sleep mode
void EN25QXXX_sleep_mode(SPI_Type *SPIx)
{
    EN25Q_operation.cs(EN25Q_operation.GPIOx,EN25Q_operation.Pin,0);
    EN25Q_operation.msg_que(SPIx,EN25Q_DEEP_POWER);
    EN25Q_operation.cs(EN25Q_operation.GPIOx,EN25Q_operation.Pin,1);
}

//在这个文件中，实现对flash的操作
//使用read，write，clear，close这四个函数实现多种功能
//open，打开flash,确定使用spi通道。
//read，实现对flash的读取数据，读取id，读取指定地址的数据，读取status
//write，实现对flash的写如操作，写入状态，向指定地址写入数据
//clear，实现擦除操作，sector，chip_erase
//close，关闭flash
  
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
}
