#include"common.h"
#include"led.h"
#include"uart.h"
#include"systemclk.h"
#include<stdio.h>

int main(int argc,const char *argv[])
{
	//初始化延时函数
	//初始化led
	//优先级分组
	//初始化串口
	//while(1);
	SysClk_PLLEN(PLLCLK_MUL_192MHz);
	uint32_t clock;
	//char *string;
	clock = GetClock_Frequency(pll);
	DelayInit();

	led_red_init();
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    GPIO_Init(HW_GPIOA, GPIO_PIN_9, GPIO_Mode_AF_PP);
    GPIO_Init(HW_GPIOA, GPIO_PIN_10, GPIO_Mode_IN_FLOATING);
 	UART_Init(HW_USART1, BAUD_115200);
    
	while(1)
	{
		if(uart_rx_status & 0x8000)
		{
			printf("clock = [%d]\r\n",clock);
			uart_rx_status = 0;
		}
	}
	
}	

