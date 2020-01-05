#include<spi_flash.h>

//实现软件上的逻辑
//open
//read
//write
//clear
//close
spi_flash_dev_t operas;

uint16_t spi_flash_open(spi_flash_dev_t *operation)
{
	//由设备端调用，初始化spi，获取设备的id号
	uint16_t device_id;
	memcpy(&operas,operation,sizeof(spi_flash_dev_t));

	operas.resetbaud(SPI_CTRL1_MCLKP_2);
	
	device_id = operas.open();
	
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
    spi_flash_clear(addr / SECSIZE * SECSIZE);
    if(bufnum > PAGESIZE)
        bufnum = PAGESIZE;
    i = operas.write(addr,buf,bufnum);
    
    return i;
}

void spi_flash_clear(uint32_t addr)
{
	//由设备端调用，擦除设备中数据
	operas.clear(addr);
}

void spi_flash_close(void)
{
	operas.release();
	memset(&operas,0,sizeof(spi_flash_dev_t));
}



