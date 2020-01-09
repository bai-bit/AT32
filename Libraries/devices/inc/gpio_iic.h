#ifndef IIC_H
#define IIC_H
#include "gpio_init.h"
#include "common.h"
#include <stdio.h>
#include <string.h>

typedef struct simulation_iic {
    void (*IIC_GPIO_INIT)(void);
    void (*IIC_SDA_OUT)(void);
    void (*IIC_SDA_IN)(void);
    void (*IIC_SCL)(uint8_t status);
    void (*IIC_SDA)(uint8_t status);
    uint16_t (*READ_SDA)(void);
}simulation_iic_ops_t;

void IIC_Init(simulation_iic_ops_t *opera);
void IIC_Start(void);
void IIC_Stop(void);
uint8_t IIC_WaitAck(void);
void IIC_Ack(void);
void IIC_NotAck(void);
void IIC_SendData(uint8_t data);
uint8_t IIC_ReceiveData(uint8_t ack);

#endif
