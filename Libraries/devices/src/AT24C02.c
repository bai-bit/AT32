#include<AT24C02.h>

uint8_t AT24CXX_check(void)
{
    uint8_t buf[] = {0xee,0xee};
    uint8_t data = 0;
    //在最后一个地址连续写入两个字节
    //根据AT24CXX的回卷特性，读取0地址的数据是否写入的数据
    //以此来判断大小。
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
    //读取指定地址上的数据长度
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
    //在指定的地址写入指定长度的数据
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
    //在指定的地址读取一个字节
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
    //在指定的地址写入一个字节
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
