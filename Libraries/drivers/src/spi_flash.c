#include<spi_flash.h>

//ʵ������ϵ��߼�
//open
//read
//write
//clear
//close
spi_flash_dev_t operas;

uint16_t spi_flash_open(spi_flash_dev_t *operation)
{
	//���豸�˵��ã���ʼ��spi����ȡ�豸��id��
	uint16_t device_id;
	memcpy(&operas,operation,sizeof(spi_flash_dev_t));

	operas.resetbaud(SPI_CTRL1_MCLKP_2);
	
	device_id = operas.open();
	
	return device_id;
}

uint32_t spi_flash_read(uint32_t addr,uint8_t *buf,uint32_t bufnum)
{
	//���豸�˵��ã���ȡ�豸�Ĵ�������
	uint32_t i = 0;
	i = operas.read(addr,buf,bufnum);
	
	return i;
}

uint32_t spi_flash_write(uint32_t addr,uint8_t *buf,uint32_t bufnum)
{
	//���豸�˵��ã����豸д������
    //����д
    uint32_t i = 0;
    spi_flash_clear(addr / SECSIZE * SECSIZE);
    if(bufnum > PAGESIZE)
        bufnum = PAGESIZE;
    i = operas.write(addr,buf,bufnum);
    
    return i;
}

void spi_flash_clear(uint32_t addr)
{
	//���豸�˵��ã������豸������
	operas.clear(addr);
}

void spi_flash_close(void)
{
	operas.release();
	memset(&operas,0,sizeof(spi_flash_dev_t));
}



