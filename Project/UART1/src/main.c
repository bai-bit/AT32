#include "common.h"
#include "led.h"
#include "uart.h"
#include "systemclk.h"
#include "exti.h"
#include<stdio.h>

int main(int argc,const char *argv[])
{
    SysClk_HSIEN();
    
	DelayInit();

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    
    RCC->APB2EN |= AFIO_ENABLEBIT;
    AFIO->MAP |= 0x4;
    GPIO_Init(HW_GPIOB, GPIO_PIN_6, GPIO_Mode_AF_PP);
    GPIO_Init(HW_GPIOB, GPIO_PIN_7, GPIO_Mode_IN_FLOATING);
 	UART_Init(HW_USART1, BAUD_115200);
    USART_ITConfig(HW_USART1, UART_INT_RDNE, ENABLE);
    NVIC_Init(USART1_IRQn, 2, 2, ENABLE);

	while(1)
	{
        printf("123");
        delayms(500);
	}
}








