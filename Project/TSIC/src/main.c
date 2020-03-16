#include "tsic506.h"
#include "systemclk.h"
#include "uart.h"
#include "common.h"
#include "gpio_init.h"
void tsic_power(uint8_t status);
uint8_t tsic_data_output(void);

tsic_oper_t tsic_opt = {
    .tsic_delayus = delayus,
    .tsic_delayms = delayms,
    .tsic_power = tsic_power,
    .tsic_data_output = tsic_data_output,
};

void tsic_power(uint8_t status)
{
    PAout(8) = status;
}

uint8_t tsic_data_output(void)
{
    return PAin(7);
}
int main(int argc,const char *argv[])
{
    SysClk_HSIEN();
    
    RCC->CTRL &= (!HSEEN_BIT);
	DelayInit();
    Tsic_Init(&tsic_opt);

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    
    RCC->APB2EN |= AFIO_ENABLEBIT;
    AFIO->MAP |= 0x4;
    GPIO_Init(HW_GPIOB, GPIO_PIN_6, GPIO_Mode_AF_PP);
    GPIO_Init(HW_GPIOB, GPIO_PIN_7, GPIO_Mode_IN_FLOATING);
 	UART_Init(HW_USART1, BAUD_115200);

    GPIO_Init(HW_GPIOA, GPIO_PIN_2, GPIO_Mode_AF_PP);
    GPIO_Init(HW_GPIOA, GPIO_PIN_3, GPIO_Mode_IN_FLOATING);
    UART_Init(HW_USART2, BAUD_115200);

    GPIO_Init(HW_GPIOA,GPIO_PIN_9,GPIO_Mode_Out_PP);
    PAout(9) = 1;
    delayms(800);
    GPIO_Init(HW_GPIOA,GPIO_PIN_8,GPIO_Mode_Out_PP);
    PAout(8) = 0;
    delayus(120);
    GPIO_Init(HW_GPIOA,GPIO_PIN_7,GPIO_Mode_IN_FLOATING);

    double data = 0;
   
	while(1)
	{
        if(read_tsic506_byte(&data))
            printf("read tsic data error\t\n");
        else
            printf("data = [%f]\r\n",data);
//        delayms(200);
	}
}

