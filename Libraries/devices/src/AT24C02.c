#include<AT24C02.h>

uint8_t AT24CXX_check(void)
{
    uint8_t buf[] = {0xee,0xee};
    uint8_t data = 0;
    //�����һ����ַ����д�������ֽ�
    //����AT24CXX�Ļؾ����ԣ���ȡ0��ַ�������Ƿ�д�������
    //�Դ����жϴ�С��
    AT24CXX_write(EEPROM_ENDADDR,buf,sizeof(buf));
    
    data = AT24CXX_readbyte(0);
    
    if(data != 0xee)
    {
        printf("check error\r\n");
        return 0;
    }
    
    return 1;
}

uint32_t AT24CXX_read(uint32_t addr,uint8_t *buf,uint32_t bufnum)
{
    //��ȡָ����ַ�ϵ����ݳ���
    uint8_t i;
    
    for(i = 0;i < bufnum;i++)
    {
        buf[i] = AT24CXX_readbyte(addr);
        addr++;
        if(addr > EEPROM_ENDADDR)
            addr = 0;
    }
    return i;
}

void AT24CXX_write(uint32_t addr,uint8_t *buf,uint32_t bufnum)
{
    //��ָ���ĵ�ַд��ָ�����ȵ�����
    uint8_t i;
    
    for(i = 0;i < bufnum;i++)
    {
        AT24CXX_writebyte(addr,buf[i]);
        addr++;
        if(addr > EEPROM_ENDADDR)
            addr = 0;
    }
}

uint8_t AT24CXX_readbyte(uint32_t addr)
{
    uint8_t temp = 0;
    //��ָ���ĵ�ַ��ȡһ���ֽ�
    IIC_Start();
    IIC_SendData(0xA0);
    if(IIC_WaitAck())
    {
        IIC_SendData(addr);
        IIC_WaitAck();
        IIC_Start();
        IIC_SendData(0xA1);
        IIC_WaitAck();
        temp = IIC_ReceiveData(0);
        IIC_Stop();		
    }
    return temp;
}

void AT24CXX_writebyte(uint32_t addr,uint8_t buf)
{
    //��ָ���ĵ�ַд��һ���ֽ�
    uint32_t delay = 100000;
    IIC_Start();
    
    IIC_SendData(0xA0);
    
    if(IIC_WaitAck())
    {
        IIC_SendData(addr);
        IIC_WaitAck();
        IIC_SendData(buf);
        IIC_WaitAck();
        IIC_Stop();
        while(delay--);
    }
}
