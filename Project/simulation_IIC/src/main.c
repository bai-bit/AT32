#include "AT24C02.h"
#include "systemclk.h"
#include "common.h"
#include "uart.h"
#include "gpio_init.h"

#define SIZE 64
#define AT24C02_TEST_ADDR 253
void simulation_iic_init(void);
void simulation_sda_out(void);
void simulation_sda_in(void);
void simulation_sda(uint8_t status);
uint16_t simulation_sda_read(void);
void simulation_scl(uint8_t status);
void delay_time(uint8_t time);
simulation_iic_ops_t ops = {
    .IIC_DELAY = delay_time,
    .IIC_GPIO_INIT = simulation_iic_init,
    .IIC_SDA_OUT = simulation_sda_out,
    .IIC_SDA_IN = simulation_sda_in,
    .IIC_SCL = simulation_scl,
    .IIC_SDA = simulation_sda,
    .READ_SDA = simulation_sda_read
};

int main(int argc,const char *argv[])
{
    SysClk_PLLEN(PLLCLK_MUL_192MHz);
    uint8_t buf[SIZE] = "ladies and gentlemen,good everyone";
    uint8_t rbuf[SIZE] = {};
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    DelayInit();
    GPIO_Init(HW_GPIOA, GPIO_PIN_9,GPIO_Mode_AF_PP);
    GPIO_Init(HW_GPIOA, GPIO_PIN_10,GPIO_Mode_IN_FLOATING);
    UART_Init(HW_USART1,BAUD_115200);
    
    IIC_Init(&ops);
    
    if(AT24CXX_check())
    {
        printf("AT24C02 is OK\r\n");
    }
    AT24CXX_write(AT24C02_TEST_ADDR,buf,sizeof(buf));
    AT24CXX_read(AT24C02_TEST_ADDR,rbuf,SIZE);
    printf("rbuf = [%s]\r\n",rbuf);
    while(1)
        continue;
}
    
//iic_test functional
void simulation_iic_init(void)
{
    GPIO_Init(HW_GPIOB,GPIO_PIN_8,GPIO_Mode_Out_PP );
    GPIO_Init(HW_GPIOB,GPIO_PIN_9,GPIO_Mode_Out_PP );
}
void simulation_sda_out(void)
{
    GPIOB->CTRLH &= 0xFFFFFF0F;GPIOB->CTRLH |= (3 << 4);
}
void simulation_sda_in(void)
{
    GPIOB->CTRLH &= 0xFFFFFF0F;GPIOB->CTRLH |= (8 << 4);
}
void simulation_sda(uint8_t status)
{
    PBout(9) = status;
}
uint16_t simulation_sda_read(void)
{
    return PBin(9);
}
void simulation_scl(uint8_t status)
{
    PBout(8) = status;
}
void delay_time(uint8_t time)
{
    delayus(time);
}

