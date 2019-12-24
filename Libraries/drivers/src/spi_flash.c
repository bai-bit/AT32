#include<spi_flash.h>

//ʵ������ϵ��߼�
//open
//read
//write
//clear
//close
spi_flash_dev_t operas;
uint16_t spi_flash_open(SPI_Type *SPIx,spi_flash_dev_t *operation)
{
	//���豸�˵��ã���ʼ��spi����ȡ�豸��id��
	uint16_t device_id;
	
	memcpy(&operas,operation,sizeof(spi_flash_dev_t));
	device_id = operas.open(SPIx);
	
	return device_id;
}

uint32_t spi_flash_read(SPI_Type *SPIx,uint32_t addr,uint8_t *buf,uint32_t bufnum,uint8_t cmd)
{
	//���豸�˵��ã���ȡ�豸�Ĵ�������
	uint32_t i = 0;
	i = operas.read(SPIx,addr,buf,bufnum,cmd);
	
	return i;
}

uint32_t spi_flash_write(SPI_Type *SPIx,uint32_t addr,uint8_t *buf,uint32_t bufnum,uint8_t cmd)
{
	//���豸�˵��ã����豸д������
	uint32_t i = 0;
	i = operas.write(SPIx,addr,buf,bufnum);
	
	return i;
}

void spi_flash_clear(SPI_Type *SPIx,uint32_t addr,uint8_t cmd)
{
	//���豸�˵��ã������豸������

	operas.clear(SPIx,addr,cmd);
}

void spi_flash_close(SPI_Type *SPIx)
{
	operas.release(SPIx);
	memset(&operas,0,sizeof(spi_flash_dev_t));
}


