#ifndef IIC_H
#define IIC_H
#include<gpio_init.h>
#include<common.h>
#include<stdio.h>

#define IIC_SDA_OUT    {GPIOB->CTRLH &= 0xFFFFFF0F;GPIOB->CTRLH |= (3 << 4);}
#define IIC_SDA_IN     {GPIOB->CTRLH &= 0xFFFFFF0F;GPIOB->CTRLH |= (8 << 4);}


#define IIC_SCL    PBout(8) 
#define IIC_SDA    PBout(9) 	 
#define READ_SDA   PBin(9)   

void IIC_Init(void);
void IIC_Start(void);
void IIC_Stop(void);
uint8_t IIC_WaitAck(void);
void IIC_Ack(void);
void IIC_NotAck(void);
void IIC_SendData(uint8_t data);
uint8_t IIC_ReceiveData(uint8_t ack);




#endif
