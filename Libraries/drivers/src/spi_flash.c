#include<spi_flash.h>

#define SECSIZE              4096
#define SECNUM               4096
#define PAGESIZE             256

#define SEND_DEFAULT_VALUE   0xFF
#define READ_DEVICE_ID       0x90
#define READ_IDENTIFICATION  0x9F
#define EN25Q_RELEASE_SLEEP  0xAB
#define EN25Q_DEEP_POWER     0xB9
#define EN25Q_FAST_READ      0x03
#define EN25Q_READ_STATUS    0x05
#define EN25Q_WRITE_ENABLE   0x06
#define EN25Q_WRITE_DISABLE  0x04
#define EN25Q_WRITE_STATUS   0x01
#define EN25Q_PAGE_PROGRAM   0x02
#define EN25Q_SECTOR_ERASE   0x20
#define EN25Q_64K_ERASE      0x08
#define EN25Q_CHIP_ERASE     0x60
#define EN25Q_BASE           0X02

static uint16_t EN25QXXX_init(void);
static void EN25QXXX_active_mode(void);
static void EN25QXXX_wait_busy(void);

//static void EN25QXXX_write_nocheck(uint32_t addr,uint8_t *buf,uint32_t bufnum);
//static uint32_t EN25QXXX_write_data(uint32_t addr,uint8_t *buf,uint32_t bufnum);
static uint32_t EN25QXXX_write_page(uint32_t addr,uint8_t *buf,uint32_t bufnum);

static uint16_t EN25QXXX_read_id(void);
static uint32_t EN25QXXX_read_data(uint32_t addr,uint8_t *buf,uint32_t bufnum);
static uint8_t EN25QXXX_read_register(void);

static void EN25QXXX_erase_sector(const uint32_t addr);
static void EN25QXXX_sleep_mode(void);

spi_flash_dev_t operas = {
    .open = EN25QXXX_init,
    .read = EN25QXXX_read_data,
    .write = EN25QXXX_write_page,
    .erase = EN25QXXX_erase_sector,
    .release = EN25QXXX_sleep_mode,
};
void spi_flash_module_init(spi_flash_dev_t *operation)
{
    memcpy(&operas,operation,sizeof(spi_flash_dev_t));
}
uint16_t spi_flash_open(void)
{
	//由设备端调用，初始化spi，获取设备的id号
    uint16_t device_id;
		
    device_id = operas.open();
    operas.device_id = device_id;
    
    return device_id;
}

uint32_t spi_flash_read(uint32_t addr,uint8_t *buf,uint32_t bufnum)
{
    //由设备端调用，读取设备的储存数据
    uint32_t i = 0;
    i = operas.read(addr,buf,bufnum);
    
    return i;
}

uint32_t spi_flash_write(uint32_t addr,uint8_t *buf,uint32_t bufnum)
{
    //有设备端调用，向设备写入数据
    //读改写
    uint32_t i = 0;
    spi_flash_erase(addr / SECSIZE * SECSIZE);
    if(bufnum > PAGESIZE)
        bufnum = PAGESIZE;
    i = operas.write(addr,buf,bufnum);
    
    return i;
}

void spi_flash_erase(uint32_t addr)
{
	//由设备端调用，擦除设备中数据
	operas.erase(addr);
}

void spi_flash_close(void)
{
	operas.release();
	memset(&operas,0,sizeof(spi_flash_dev_t));
}

EN25Q_dev_t EN25Q_operation;

//initialization EN25Q
void EN25Q_module_init(EN25Q_dev_t *opers)
{
    memcpy(&EN25Q_operation,opers,sizeof(EN25Q_dev_t)); 
    
}

static uint16_t EN25QXXX_init(void)
{
    uint16_t EN25Q_ID = 0;
    
    EN25QXXX_active_mode();
    EN25Q_ID |= EN25QXXX_read_id();
    return EN25Q_ID;
}


static uint16_t EN25QXXX_read_id(void)
{
    //chip select low
    //send read instruction
    //send 00h thrice
    //then receive data by send ffh
    uint16_t ret = 0;

    EN25Q_operation.cs(0);
    EN25Q_operation.xfer_data(READ_DEVICE_ID);
    EN25Q_operation.xfer_data(0x00);
    EN25Q_operation.xfer_data(0x00);
    EN25Q_operation.xfer_data(0x00);
    ret |= EN25Q_operation.xfer_data(SEND_DEFAULT_VALUE) << 8;
    ret |= EN25Q_operation.xfer_data(SEND_DEFAULT_VALUE);
    EN25Q_operation.cs(1);
    
    return ret;
}


//read data for en25q
static uint32_t EN25QXXX_read_data(uint32_t addr, uint8_t *rbuf, uint32_t bufnum)
{
    //读ID,读status,读data
    uint32_t i = 0;
    
    EN25Q_operation.cs(0);
	
    EN25Q_operation.xfer_data(EN25Q_FAST_READ);
    EN25Q_operation.xfer_data((uint8_t)((addr) >> 16));
    EN25Q_operation.xfer_data((uint8_t)((addr) >> 8));
    EN25Q_operation.xfer_data((uint8_t)(addr));
    
    for(i = 0;i < bufnum;i++)
        rbuf[i] = EN25Q_operation.xfer_data(SEND_DEFAULT_VALUE);
    
    EN25Q_operation.cs(1);
    return sizeof(rbuf);
}
//read status register for en25q
static uint8_t EN25QXXX_read_register(void)
{
    uint8_t status = 0;
    
    EN25Q_operation.cs(0);
    
    EN25Q_operation.xfer_data(EN25Q_READ_STATUS);
    status = EN25Q_operation.xfer_data(SEND_DEFAULT_VALUE);
    
    EN25Q_operation.cs(1);
    
    return status;
}
//write enable
static void EN25QXXX_write_enable(void)
{
    EN25Q_operation.cs(0);
    EN25Q_operation.xfer_data(EN25Q_WRITE_ENABLE);
    EN25Q_operation.cs(1);    
}
//write disable
static void EN25QXXX_write_disable(void)
{
    EN25Q_operation.cs(0);
    EN25Q_operation.xfer_data(EN25Q_WRITE_DISABLE);
    EN25Q_operation.cs(1);    
}
//write data to en25q
//uint8_t secbuf[SECSIZE];
//static uint32_t EN25QXXX_write_data(uint32_t addr,uint8_t *buf,uint32_t bufnum)
//{
//    uint32_t m = 0;
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
//    return m;
//}

//256 or >256
static uint32_t EN25QXXX_write_page(uint32_t addr,uint8_t *buf,uint32_t bufnum)
{
    uint16_t i;
    
    EN25QXXX_write_enable();
    EN25QXXX_wait_busy();
    
    EN25Q_operation.cs(0);
    EN25Q_operation.xfer_data(EN25Q_PAGE_PROGRAM);
    EN25Q_operation.xfer_data((uint8_t)((addr) >> 16));
    EN25Q_operation.xfer_data((uint8_t)((addr) >> 8));
    EN25Q_operation.xfer_data((uint8_t)(addr));
    
    for(i = 0;i < bufnum;i++)
        EN25Q_operation.xfer_data(buf[i]);
    
    EN25Q_operation.cs(1);
    EN25QXXX_wait_busy();
    return i;
}

//void EN25QXXX_write_nocheck(uint32_t addr,uint8_t *buf,uint32_t bufnum)
//{
//    //检查写入的地址，在某一个页内的偏移地址
//    //比较页内剩余空间和实际写入的字节数，决定传递的参数
//    //如果实际写入的字节数在一个页内写不完，就重复调用写入函数，前提是将每次传递的参数处理好
//    uint32_t pagesurplus = 0;
//    
//    pagesurplus = PAGESIZE - ((addr) % PAGESIZE);
//    if(pagesurplus >= bufnum)
//        pagesurplus = bufnum;
//    EN25QXXX_write_page((addr),buf,pagesurplus);
//    while(pagesurplus < bufnum)
//    {
//        buf = buf + pagesurplus;
//        addr = (addr) + pagesurplus;
//        bufnum = bufnum - pagesurplus;
//        if(bufnum >= PAGESIZE)
//            pagesurplus = PAGESIZE;
//        else
//            pagesurplus = bufnum;
//		
//        EN25QXXX_write_page((addr),buf,pagesurplus);
//    }
//}

static void EN25QXXX_erase_sector(const uint32_t addr)
{
    uint32_t temp = addr;
    
    EN25QXXX_write_enable();
    EN25QXXX_wait_busy();
    
    EN25Q_operation.cs(0);
    
    EN25Q_operation.xfer_data(EN25Q_SECTOR_ERASE);
    EN25Q_operation.xfer_data((uint8_t)((temp) >> 16));
    EN25Q_operation.xfer_data((uint8_t)((temp) >> 8));
    EN25Q_operation.xfer_data((uint8_t)(temp));
    
    EN25Q_operation.cs(1);  
    
    EN25QXXX_wait_busy();
}
//chip erase
static void EN25QXXX_chip_erase(void)
{
    EN25QXXX_write_enable();
    EN25QXXX_wait_busy();
    
    EN25Q_operation.cs(0);
    EN25Q_operation.xfer_data(EN25Q_CHIP_ERASE);
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
    EN25Q_operation.xfer_data(EN25Q_RELEASE_SLEEP);
    EN25Q_operation.cs(1);
}

//sleep mode
static void EN25QXXX_sleep_mode(void)
{
    EN25QXXX_write_disable();
    EN25Q_operation.cs(0);
    EN25Q_operation.xfer_data(EN25Q_DEEP_POWER);
    EN25Q_operation.cs(1);
}



