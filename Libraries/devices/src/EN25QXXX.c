#include<EN25QXXX.h>

EN25Q_dev_t EN25Q_operation;

//initialization EN25Q
void EN25Q_module_init(EN25Q_dev_t *opers)
{
    memcpy(&EN25Q_operation,opers,sizeof(EN25Q_dev_t)); 
    
}
uint16_t EN25QXXX_init(void)
{
    uint16_t EN25Q_ID = 0;
    
    EN25QXXX_active_mode();
    EN25Q_ID |= EN25QXXX_readID();
    return EN25Q_ID;
}


uint16_t EN25QXXX_readID(void)
{
    //chip select low
    //send read instruction
    //send 00h thrice
    //then receive data by send ffh
    uint16_t ret = 0;

    EN25Q_operation.cs(0);
    EN25Q_operation.send_data(EN25Q_operation.channel,READ_DEVICE_ID);
    EN25Q_operation.send_data(EN25Q_operation.channel,0x00);
    EN25Q_operation.send_data(EN25Q_operation.channel,0x00);
    EN25Q_operation.send_data(EN25Q_operation.channel,0x00);
    ret |= EN25Q_operation.send_data(EN25Q_operation.channel,SEND_DEFAULT_VALUE) << 8;
    ret |= EN25Q_operation.send_data(EN25Q_operation.channel,SEND_DEFAULT_VALUE);
    EN25Q_operation.cs(1);
    
    return ret;
}


//read data for en25q
uint32_t EN25QXXX_read_data(uint32_t addr,uint8_t *rbuf,uint32_t bufnum)
{
    //读ID,读status,读data
    uint32_t i = 0;
    
    EN25Q_operation.cs(0);
	
    EN25Q_operation.send_data(EN25Q_operation.channel,EN25Q_FAST_READ);
    EN25Q_operation.send_data(EN25Q_operation.channel,(uint8_t)((addr) >> 16));
    EN25Q_operation.send_data(EN25Q_operation.channel,(uint8_t)((addr) >> 8));
    EN25Q_operation.send_data(EN25Q_operation.channel,(uint8_t)(addr));
    
    for(i = 0;i < bufnum;i++)
        rbuf[i] = EN25Q_operation.send_data(EN25Q_operation.channel,SEND_DEFAULT_VALUE);
    
    EN25Q_operation.cs(1);
    return sizeof(rbuf);
}
//read status register for en25q
static uint8_t EN25QXXX_read_register(void)
{
    uint8_t status = 0;
    
    EN25Q_operation.cs(0);
    
    EN25Q_operation.send_data(EN25Q_operation.channel,EN25Q_READ_STATUS);
    status = EN25Q_operation.send_data(EN25Q_operation.channel,SEND_DEFAULT_VALUE);
    
    EN25Q_operation.cs(1);
    
    return status;
}
//write enable
static void EN25QXXX_write_enable(void)
{
    EN25Q_operation.cs(0);
    EN25Q_operation.send_data(EN25Q_operation.channel,EN25Q_WRITE_ENABLE);
    EN25Q_operation.cs(1);    
}
//write disable
static void EN25QXXX_write_disable(void)
{
    EN25Q_operation.cs(0);
    EN25Q_operation.send_data(EN25Q_operation.channel,EN25Q_WRITE_DISABLE);
    EN25Q_operation.cs(1);    
}
//write data to en25q
//uint8_t secbuf[SECSIZE];
uint32_t EN25QXXX_write_data(uint32_t addr,uint8_t *buf,uint32_t bufnum)
{
    uint32_t m = 0;
//    uint32_t sectorid = 0,secoffset = 0,secsurplus = 0;
//    uint32_t i = 0;
//    uint8_t *temp_buf = secbuf;
//    
//    if(addr < EN25Q_BASE && addr > SECSIZE * SECNUM)
//        return 0;
//    if(bufnum > SECSIZE * SECNUM - addr)
//        bufnum = SECSIZE * SECNUM - addr;
//    sectorid = addr / SECSIZE;
//    secoffset = addr % SECSIZE;
//    secsurplus = SECSIZE - secoffset;
//    if(secsurplus > bufnum)
//        secsurplus = bufnum;
//    m = secsurplus;
//    
//    while(1)
//    {
//        EN25QXXX_read_data(EN25Q_operation.channel,sectorid * SECSIZE,temp_buf,SECSIZE);
//        for(i = 0;i < secsurplus;i++)
//            if(temp_buf[secoffset + i] != 0xFF)
//                break;
//    
//        if(i < secsurplus)
//        {
//            EN25QXXX_erase_sector(EN25Q_operation.channel,sectorid * SECSIZE);
//    
//            for(i = 0;i < secsurplus;i++)
//                temp_buf[secoffset + i] = buf[i];
//			
//            EN25QXXX_write_nocheck(EN25Q_operation.channel,sectorid * SECSIZE,temp_buf,SECSIZE);
//        }
//        else
//            EN25QXXX_write_nocheck(EN25Q_operation.channel,addr,buf,bufnum);
//			
//        if(secsurplus == bufnum)
//            break;
//        else
//        {
//            bufnum -= secsurplus;
//            addr += secsurplus;
//            buf += secsurplus;
//            sectorid++;
//            secoffset = 0;
//            if(bufnum > SECSIZE)
//                secsurplus = SECSIZE;
//            else
//                secsurplus = bufnum;
//            m += secsurplus;
//        }
//    }
    return m;
}

//256 or >256
uint32_t EN25QXXX_write_page(uint32_t addr,uint8_t *buf,uint32_t bufnum)
{
    uint16_t i;
    
    EN25QXXX_write_enable();
    EN25QXXX_wait_busy();
    
    EN25Q_operation.cs(0);
    EN25Q_operation.send_data(EN25Q_operation.channel,EN25Q_PAGE_PROGRAM);
    EN25Q_operation.send_data(EN25Q_operation.channel,(uint8_t)((addr) >> 16));
    EN25Q_operation.send_data(EN25Q_operation.channel,(uint8_t)((addr) >> 8));
    EN25Q_operation.send_data(EN25Q_operation.channel,(uint8_t)(addr));
    
    for(i = 0;i < bufnum;i++)
        EN25Q_operation.send_data(EN25Q_operation.channel,buf[i]);
    
    EN25Q_operation.cs(1);
    EN25QXXX_wait_busy();
    return i;
}

void EN25QXXX_write_nocheck(uint32_t addr,uint8_t *buf,uint32_t bufnum)
{
    //检查写入的地址，在某一个页内的偏移地址
    //比较页内剩余空间和实际写入的字节数，决定传递的参数
    //如果实际写入的字节数在一个页内写不完，就重复调用写入函数，前提是将每次传递的参数处理好
    uint32_t pagesurplus = 0;
    
    pagesurplus = PAGESIZE - ((addr) % PAGESIZE);
    if(pagesurplus >= bufnum)
        pagesurplus = bufnum;
    EN25QXXX_write_page((addr),buf,pagesurplus);
    while(pagesurplus < bufnum)
    {
        buf = buf + pagesurplus;
        addr = (addr) + pagesurplus;
        bufnum = bufnum - pagesurplus;
        if(bufnum >= PAGESIZE)
            pagesurplus = PAGESIZE;
        else
            pagesurplus = bufnum;
		
        EN25QXXX_write_page((addr),buf,pagesurplus);
    }
}

void EN25QXXX_erase_sector(const uint32_t addr)
{
    uint32_t temp = addr;
    
    EN25QXXX_write_enable();
    EN25QXXX_wait_busy();
    
    EN25Q_operation.cs(0);
    
    EN25Q_operation.send_data(EN25Q_operation.channel,EN25Q_SECTOR_ERASE);
    EN25Q_operation.send_data(EN25Q_operation.channel,(uint8_t)((temp) >> 16));
    EN25Q_operation.send_data(EN25Q_operation.channel,(uint8_t)((temp) >> 8));
    EN25Q_operation.send_data(EN25Q_operation.channel,(uint8_t)(temp));
    
    EN25Q_operation.cs(1);  
    
    EN25QXXX_wait_busy();
}
//chip erase
void EN25QXXX_chip_erase(void)
{
    EN25QXXX_write_enable();
    EN25QXXX_wait_busy();
    
    EN25Q_operation.cs(0);
    EN25Q_operation.send_data(EN25Q_operation.channel,EN25Q_CHIP_ERASE);
    EN25Q_operation.cs(1);
    EN25QXXX_wait_busy();
}
//wait busy
static void EN25QXXX_wait_busy(void)
{
    while((EN25QXXX_read_register() & 0x01) == 0x01) 
        continue;    
}

//active mode
static void EN25QXXX_active_mode(void)
{
    EN25Q_operation.cs(0);
    EN25Q_operation.send_data(EN25Q_operation.channel,EN25Q_RELEASE_SLEEP);
    EN25Q_operation.cs(1);
}

//sleep mode
void EN25QXXX_sleep_mode(void)
{
    EN25Q_operation.cs(0);
    EN25Q_operation.send_data(EN25Q_operation.channel,EN25Q_DEEP_POWER);
    EN25Q_operation.cs(1);
}
void EN25QXXX_baud(uint32_t baud)
{
    EN25Q_operation.reset_baud(EN25Q_operation.channel,baud);
}
