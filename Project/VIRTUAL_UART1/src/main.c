/************************************************************************************/
/****这个模拟串口收发：由于延时函数只能延时整数，小数部分无法实现，所以该代码只能****/
/****在168M、176M、192M和200M的时钟频率，波特率为115200的环境下，数据准确接收和发****/
/****在模拟串口发送时，不使用接收时，在这三个时钟频率下，可以满足460800、115200和****/
/****9600这三个波特率的成功发送，如果需要使用其他的波特率或者是更小的时钟频率，则****/
/****需要更改延时变量uart_delayus和uart_delayus_again的计算方法，或者添加空指令，****/
/****达到延时1微秒以下的时间。                                                   ****/
/****                                                                            ****/
/************************************************************************************/
#include "common.h"
#include "led.h"
#include "uart.h"
#include "virtual_uart.h"
#include "systemclk.h"


extern u8 uart_read_buf[UART_SIZE];
extern uint16_t uart_read_status;


int main(int argc, const char *argv[])
{
	SysClk_PLLEN(PLLCLK_MUL_192MHz);
	DelayInit();

	led_red_init();
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
 
	uart_virtual(115200);

	while(1)
	{
		if(uart_read_status & UART_STATUS_NLINE)
		{
			uartsend_data(HW_USART1);
			uart_read_status = 0;
		}
	}
}	

