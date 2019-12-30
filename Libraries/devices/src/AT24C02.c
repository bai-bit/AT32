//read
//write
//device addr  0xA0;
#include<AT24C02.H>

void AT24CXX_Init(void)
{
	iic_init();	
}

uint8_t AT24CXX_check(void)
{
    uint8_t buf[2] = {0x01,0x01};
    uint8_t data = 0;
	//�����һ����ַ����д�������ֽ�
	//����AT24CXX�Ļؾ����ԣ���ȡ0��ַ�������Ƿ�д�������
	//�Դ����жϴ�С��
   
//	AT24CXX_write(255,buf,1);
    AT24CXX_writebyte(1,0x55);
    printf("4");
    data = AT24CXX_readbyte(1);
    printf("5");
    if(data != 0x55)
        return 0;
    printf("6");
    return 1;
	
}

uint32_t AT24CXX_read(uint32_t addr,uint8_t *buf,uint32_t bufnum)
{
	//��ȡָ����ַ�ϵ����ݳ���
    uint8_t i;
 
    for(i = 0;i < bufnum;i++)
        buf[i] = AT24CXX_readbyte(addr);
    
	return i;
}

void AT24CXX_write(uint32_t addr,uint8_t *buf,uint32_t bufnum)
{
	//��ָ���ĵ�ַд��ָ�����ȵ�����
    uint8_t i;
    for(i = 0;i < bufnum;i++)
        AT24CXX_writebyte(addr,buf[i]);
	
}

uint8_t AT24CXX_readbyte(uint32_t addr)
{
    uint8_t temp = 0;
	//��ָ���ĵ�ַ��ȡһ���ֽ�
	iic_start();
	iic_send_data(0xA0);
	if(iic_wait_ack())
	{
        printf("ik");
		iic_send_data(addr);
        iic_wait_ack();
        iic_start();
        iic_send_data(0xA1);
        iic_wait_ack();
        temp = iic_receive_data(0);
        iic_stop();		
	}
    
    return temp;
}

void AT24CXX_writebyte(uint32_t addr,uint8_t buf)
{
	//��ָ���ĵ�ַд��һ���ֽ�
  
    iic_start();
    
    iic_send_data(0xA0);
  
    if(iic_wait_ack())
    {
        
        iic_send_data(addr);
        
        iic_wait_ack();
        
        iic_send_data(buf);
        iic_wait_ack();
        iic_stop();
        delayms(5);
    }
}
