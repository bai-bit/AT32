//simulation iic
#include<gpio_iic.h>

static simulation_iic_ops_t ops;
void IIC_Init(simulation_iic_ops_t *opera)
{
    //initialization iic
    //initialization gpio
    //SDA PB9   SCL  PB8
    memcpy(&ops,opera,sizeof(simulation_iic_ops_t));
    ops.IIC_GPIO_INIT();
    ops.IIC_SCL(1);
    ops.IIC_SDA(1);
}

void IIC_Start(void)
{
    ops.IIC_SDA_OUT();
    
    ops.IIC_SDA(1);
    ops.IIC_SCL(1);
    ops.IIC_DELAY(5);
    ops.IIC_SDA(0);
    ops.IIC_DELAY(5);
    ops.IIC_SCL(0);
}

void IIC_Stop(void)
{
    ops.IIC_SDA_OUT();
    
    ops.IIC_SCL(0);
    ops.IIC_SDA(0);
    ops.IIC_DELAY(5);
    ops.IIC_SCL(1);
    
    ops.IIC_SDA(1);
    ops.IIC_DELAY(5);
}

uint8_t IIC_WaitAck(void)
{
    uint32_t timeout = 0x01000;
    uint8_t ret = 0;
    ops.IIC_SDA_IN();
    
    ops.IIC_SDA(1);
    ops.IIC_DELAY(2);
    ops.IIC_SCL(1);
    ops.IIC_DELAY(2);
    while(ops.READ_SDA())
    {
        timeout--;
        if(timeout == 0)
        {
            IIC_Stop();
            return 0;
        }
        printf("B");
    }
    ops.IIC_SCL(0);
    ret = 1;
    return ret;
}

void IIC_Ack(void)
{
    ops.IIC_SCL(0);
    ops.IIC_SDA_OUT();
    
    ops.IIC_DELAY(2);
    ops.IIC_SDA(0);
    ops.IIC_DELAY(2);
    ops.IIC_SCL(1);
    ops.IIC_DELAY(5);
    ops.IIC_SDA(0);
}

void IIC_NotAck(void)
{
    ops.IIC_SCL(0);
    ops.IIC_SDA_OUT();
    
    ops.IIC_DELAY(2);
    ops.IIC_SDA(1);
    ops.IIC_DELAY(2);
    ops.IIC_SCL(1);
    ops.IIC_DELAY(5);
    ops.IIC_SCL(0);
}

void IIC_SendData(uint8_t data)
{
    uint8_t i = 0;
    
    ops.IIC_SDA_OUT();
    ops.IIC_SCL(0);
    ops.IIC_DELAY(2);
    for(i = 0;i < 8;i++)
    {
        if((data & 0x80) >> 7)
            ops.IIC_SDA(1);
        else
            ops.IIC_SDA(0);
        data <<= 1;
        ops.IIC_DELAY(2);
        ops.IIC_SCL(1);
        ops.IIC_DELAY(2);
        ops.IIC_SCL(0);
        ops.IIC_DELAY(2);
    }
}

uint8_t IIC_ReceiveData(uint8_t ack)
{
    uint8_t ret = 0;
    uint8_t i = 0;
    ops.IIC_SDA_IN();
    for(i = 0;i < 8;i++)
    {
        ops.IIC_SCL(0);
        ops.IIC_DELAY(2);
        ops.IIC_SCL(1);
        ops.IIC_DELAY(2);
        ret <<= 1;
        if(ops.READ_SDA())
            ret++;
        ops.IIC_DELAY(2);
    }
    if(ack)
        IIC_Ack();
    else
        IIC_NotAck();
    return ret;
}
