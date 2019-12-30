//simulation iic
#include<iic.h>

void iic_init(void)
{
	//initialization iic
	//initialization gpio
	//SDA PB9   SCL  PB8
	
	GPIO_Init(HW_GPIOB,GPIO_PIN_8,GPIO_Speed_50MHz,GPIO_Mode_Out_PP );
	GPIO_Init(HW_GPIOB,GPIO_PIN_9,GPIO_Speed_50MHz,GPIO_Mode_Out_PP );
	
	GPIO_PinWrite(HW_GPIOB,GPIO_PIN_8,1);
	GPIO_PinWrite(HW_GPIOB,GPIO_PIN_9,1);
}

void iic_start(void)
{
	IIC_SDA_OUT;
	delayus(2);
	IIC_SDA = 1;
	IIC_SCL = 1;
	delayus(5);
	IIC_SDA = 0;
	delayus(5);
	IIC_SCL = 0;
}

void iic_stop(void)
{
	IIC_SDA_OUT;
	delayus(2);
	IIC_SCL = 0;
	IIC_SDA = 0;
	delayus(2);
	IIC_SCL = 1;
	delayus(5);
	IIC_SDA = 1;
	delayus(5);
}

uint8_t iic_wait_ack(void)
{
	uint32_t timeout = 0x01000;
	uint8_t ret = 0;
    IIC_SDA_IN;

	IIC_SDA = 1;
	delayus(2);
	IIC_SCL = 1;
	delayus(2);
  	while(READ_SDA)
	{
		timeout--;
		if(timeout == 0)
		{
			iic_stop();
			return 0;
		}
        printf("B");
	}
	IIC_SCL = 0;
	ret = 1;
	return ret;
}

void iic_ack(void)
{
	IIC_SDA_OUT;
	IIC_SCL = 0;
	delayus(2);
	IIC_SDA = 0;
	delayus(2);
	IIC_SCL = 1;
	delayus(5);
	IIC_SDA_IN;
}

void iic_notack(void)
{
	IIC_SDA_OUT;
	IIC_SCL = 0;
	delayus(2);
	IIC_SDA = 1;
	delayus(2);
	IIC_SCL = 1;
	delayus(5);
	IIC_SDA_IN;
}

void iic_send_data(uint8_t data)
{
	uint8_t i = 0;
	
	IIC_SDA_OUT;
	IIC_SCL = 0;
	delayus(2);
	IIC_SDA = 1;
	delayus(2);
	for(i = 0;i < 8;i++)
	{
		if((data & 0x80) >> 7)
			IIC_SDA = 1;
		else
			IIC_SDA = 0;
		delayus(2);
		IIC_SCL = 1;
		delayus(2);
		IIC_SCL = 0;
		delayus(2);
		data <<= 1;
	}
}

uint8_t iic_receive_data(uint8_t ack)
{
	uint8_t ret = 0;
	uint8_t i = 0;
	IIC_SDA_IN;
	for(i = 0;i < 8;i++)
	{
		IIC_SCL = 0;
		delayus(2);
		IIC_SCL = 1;
		delayus(2);
		if(READ_SDA)
			ret |= 1 << i;
	}
	if(ack)
		iic_ack();
	else
		iic_notack();
	
	return ret;
}
