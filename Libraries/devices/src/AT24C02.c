//read
//write
//device addr  0xA0;
#include<AT24C02.H>

void AT24CXX_Init(void)
{
	iic_init();	
}

void AT24CXX_check(void)
{
	//在最后一个地址连续写入两个字节
	//根据AT24CXX的回卷特性，读取0地址的数据是否写入的数据
	//以此来判断大小。
	
	
}

void AT24CXX_read(uint32_t addr,uint8_t *buf,uint32_t bufnum)
{
	//读取指定地址上的数据长度
	
}

void AT24CXX_write(uint32_t addr,uint8_t *buf,uint32_t bufnum)
{
	//在指定的地址写入指定长度的数据
	
}

void AT24CXX_readbyte(uint32_t addr,uint8_t *buf)
{
	//在指定的地址读取一个字节
	iic_start();
	iic_send_data(0xA0);
	if(iic_wait_ack())
	{
		iic_send_data(addr);
		
	}
}

uint8_t *AT24CXX_writebyte(uint32_t addr,uint8_t *buf)
{
	//在指定的地址写入一个字节

}