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
	//�����һ����ַ����д�������ֽ�
	//����AT24CXX�Ļؾ����ԣ���ȡ0��ַ�������Ƿ�д�������
	//�Դ����жϴ�С��
	
	
}

void AT24CXX_read(uint32_t addr,uint8_t *buf,uint32_t bufnum)
{
	//��ȡָ����ַ�ϵ����ݳ���
	
}

void AT24CXX_write(uint32_t addr,uint8_t *buf,uint32_t bufnum)
{
	//��ָ���ĵ�ַд��ָ�����ȵ�����
	
}

void AT24CXX_readbyte(uint32_t addr,uint8_t *buf)
{
	//��ָ���ĵ�ַ��ȡһ���ֽ�
	iic_start();
	iic_send_data(0xA0);
	if(iic_wait_ack())
	{
		iic_send_data(addr);
		
	}
}

uint8_t *AT24CXX_writebyte(uint32_t addr,uint8_t *buf)
{
	//��ָ���ĵ�ַд��һ���ֽ�

}