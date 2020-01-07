#include<spi_flash.h>

#define SECSIZE              4096
#define PAGESIZE             256
#define SEND_DEFAULT_VALUE   0xFF
#define READ_DEVICE_ID       0x90
#define READ_IDENTIFICATION  0x9F
#define FLASH_RELEASE_SLEEP  0xAB
#define FLASH_DEEP_POWER     0xB9
#define FLASH_FAST_READ      0x03
#define FLASH_READ_STATUS    0x05
#define FLASH_WRITE_ENABLE   0x06
#define FLASH_WRITE_DISABLE  0x04
#define FLASH_WRITE_STATUS   0x01
#define FLASH_PAGE_PROGRAM   0x02
#define FLASH_SECTOR_ERASE   0x20
#define FLASH_CHIP_ERASE     0x60

#define FLASH_TIMEOUT_WEIRTE 0x5000
#define FLASH_TIMEOUT_ERASE  0x50000000


static void spi_flash_active_mode(void);
static void spi_flash_wait_busy(uint32_t timeout);

static void spi_flash_write_enable(void);
static uint8_t spi_flash_read_register(void);

static void spi_flash_write_disable(void);

spi_flash_dev_t ops;
void spi_flash_init(spi_flash_dev_t *opers)
{
    memcpy(&ops,opers,sizeof(ops)); 
}

uint16_t spi_flash_open(void)
{
    //由设备端调用，初始化spi，获取设备的id号
    uint16_t device_id;
    
	spi_flash_active_mode();	
    ops.cs(0);
    ops.xfer_data(READ_DEVICE_ID);
    ops.xfer_data(0x00);
    ops.xfer_data(0x00);
    ops.xfer_data(0x00);
    device_id |= ops.xfer_data(SEND_DEFAULT_VALUE) << 8;
    device_id |= ops.xfer_data(SEND_DEFAULT_VALUE);
    ops.cs(1);
    ops.device_id = device_id;
    
    return device_id;
}

uint32_t spi_flash_read(uint32_t addr, uint8_t *buf, uint32_t bufnum)
{
    //由设备端调用，读取设备的储存数据
    uint32_t i = 0;

    ops.cs(0);
    
    ops.xfer_data(FLASH_FAST_READ);
    ops.xfer_data((uint8_t)((addr) >> 16));
    ops.xfer_data((uint8_t)((addr) >> 8));
    ops.xfer_data((uint8_t)(addr));
    
    for(i = 0;i < bufnum;i++)
        buf[i] = ops.xfer_data(SEND_DEFAULT_VALUE);
    
    ops.cs(1);
    return sizeof(buf);
}

uint32_t spi_flash_write(uint32_t addr, uint8_t *buf, uint32_t bufnum)
{
    //有设备端调用，向设备写入数据
    //读改写
    uint32_t i = 0;
    spi_flash_erase(addr / SECSIZE * SECSIZE);
    if(bufnum > PAGESIZE)
        bufnum = PAGESIZE;

    spi_flash_write_enable();
    spi_flash_wait_busy(FLASH_TIMEOUT_WEIRTE);
    
    ops.cs(0);
    ops.xfer_data(FLASH_PAGE_PROGRAM);
    ops.xfer_data((uint8_t)((addr) >> 16));
    ops.xfer_data((uint8_t)((addr) >> 8));
    ops.xfer_data((uint8_t)(addr));
    
    for(i = 0;i < bufnum;i++)
        ops.xfer_data(buf[i]);
    
    ops.cs(1);
    spi_flash_wait_busy(FLASH_TIMEOUT_WEIRTE);
    return i;
}

void spi_flash_erase(uint32_t addr)
{
	//由设备端调用，擦除设备中数据
    uint32_t temp = addr;
    
    spi_flash_write_enable();
    spi_flash_wait_busy(FLASH_TIMEOUT_WEIRTE);
    
    ops.cs(0);
    
    ops.xfer_data(FLASH_SECTOR_ERASE);
    ops.xfer_data((uint8_t)((temp) >> 16));
    ops.xfer_data((uint8_t)((temp) >> 8));
    ops.xfer_data((uint8_t)(temp));
    
    ops.cs(1);  
    
    spi_flash_wait_busy(FLASH_TIMEOUT_ERASE);
}

void spi_flash_close(void)
{
	spi_flash_write_disable();
    ops.cs(0);
    ops.xfer_data(FLASH_DEEP_POWER);
    ops.cs(1);
}


//read status register for en25q
static uint8_t spi_flash_read_register(void)
{
    uint8_t status = 0;
    
    ops.cs(0);
    
    ops.xfer_data(FLASH_READ_STATUS);
    status = ops.xfer_data(SEND_DEFAULT_VALUE);
    
    ops.cs(1);
    
    return status;
}
//write enable
static void spi_flash_write_enable(void)
{
    ops.cs(0);
    ops.xfer_data(FLASH_WRITE_ENABLE);
    ops.cs(1);    
}
//write disable
static void spi_flash_write_disable(void)
{
    ops.cs(0);
    ops.xfer_data(FLASH_WRITE_DISABLE);
    ops.cs(1);    
}

//chip erase
static void spi_flash_chip_erase(void)
{
    spi_flash_write_enable();
    spi_flash_wait_busy(FLASH_TIMEOUT_WEIRTE);
    
    ops.cs(0);
    ops.xfer_data(FLASH_CHIP_ERASE);
    ops.cs(1);
    spi_flash_wait_busy(FLASH_TIMEOUT_WEIRTE);
}
//wait busy
static void spi_flash_wait_busy(uint32_t timeout)
{
    while(spi_flash_read_register() & 0x1) 
        continue;    
}

//active mode
static void spi_flash_active_mode(void)
{
    ops.cs(0);
    ops.xfer_data(FLASH_RELEASE_SLEEP);
    ops.cs(1);
}





