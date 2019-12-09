#include<systick.h>
#include<led.h>
#include<uart.h>
#include<systemclk.h>

int main(int argc,const char *argv[])
{
	//初始化延时函数
	//初始化led
	//优先级分组
	//初始化串口
	//while(1);
	sysclk_PLLEN(PLLCLK_MUL_192MHz);
	uint32_t clock;
	char *string;
	clock = getclock_frequency(pll);
	DelayInit();

	led_red_init();
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
 	uart1_init(115200);

	while(1)
	{
		
		if(uart_rx_status & 0x4000)
		{
//		string = itoa(clock);
//		log_uart(HW_USART1,string);
			printf("clock = [%d]\r\n",clock);
			uart_rx_status = 0;
		}
	//	log_uart(HW_USART1,"ABC");
	}
	
}	

