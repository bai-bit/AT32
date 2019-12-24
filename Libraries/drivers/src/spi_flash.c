#include<spi_flash.h>

//实现软件上的逻辑
//open
//read
//write
//clear
//close
spi_flash_dev_t operas;
uint16_t spi_flash_open(SPI_Type *SPIx,spi_flash_dev_t *operation)
{
	//由设备端调用，初始化spi，获取设备的id号
	uint16_t device_id;
	
	memcpy(&operas,operation,sizeof(spi_flash_dev_t));
	device_id = operas.open(SPIx);
	
	return device_id;
}

uint32_t spi_flash_read(SPI_Type *SPIx,uint32_t addr,uint8_t *buf,uint32_t bufnum,uint8_t cmd)
{
	//由设备端调用，读取设备的储存数据
	uint32_t i = 0;
	i = operas.read(SPIx,addr,buf,bufnum,cmd);
	
	return i;
}

uint32_t spi_flash_write(SPI_Type *SPIx,uint32_t addr,uint8_t *buf,uint32_t bufnum,uint8_t cmd)
{
	//有设备端调用，向设备写入数据
	uint32_t i = 0;
	i = operas.write(SPIx,addr,buf,bufnum);
	
	return i;
}

void spi_flash_clear(SPI_Type *SPIx,uint32_t addr,uint8_t cmd)
{
	//由设备端调用，擦除设备中数据

	operas.clear(SPIx,addr,cmd);
}

void spi_flash_close(SPI_Type *SPIx)
{
	operas.release(SPIx);
	memset(&operas,0,sizeof(spi_flash_dev_t));
}


