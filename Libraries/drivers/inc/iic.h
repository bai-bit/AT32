//simulation sequence to iic
//start signal
//stop signal

//answer signal
//send byte
//receiv
#ifndef IIC_H
#define IIC_H
#include<gpio_init.h>
#include<systick.h>
#include<stdio.h>

#define IIC_SDA_OUT    {GPIOB->CTRLH &= 0xFFFFFF0F;GPIOB->CTRLH |= (3 << 4);}
#define IIC_SDA_IN     {GPIOB->CTRLH &= 0xFFFFFF0F;GPIOB->CTRLH |= (8 << 4);}


#define IIC_SCL    PBout(8) 
#define IIC_SDA    PBout(9) 	 
#define READ_SDA   PBin(9)   

void iic_init(void);
void iic_start(void);
void iic_stop(void);
uint8_t iic_wait_ack(void);
void iic_ack(void);
void iic_notack(void);
void iic_send_data(uint8_t data);
uint8_t iic_receive_data(uint8_t ack);




#endif
