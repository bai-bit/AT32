/************************************************************************************/
/****���ģ�⴮���շ���������ʱ����ֻ����ʱ������С�������޷�ʵ�֣����Ըô���ֻ��****/
/****��168M��176M��192M��200M��ʱ��Ƶ�ʣ�������Ϊ115200�Ļ����£�����׼ȷ���պͷ�****/
/****��ģ�⴮�ڷ���ʱ����ʹ�ý���ʱ����������ʱ��Ƶ���£���������460800��115200��****/
/****9600�����������ʵĳɹ����ͣ������Ҫʹ�������Ĳ����ʻ����Ǹ�С��ʱ��Ƶ�ʣ���****/
/****��Ҫ������ʱ����uart_delayus��uart_delayus_again�ļ��㷽����������ӿ�ָ�****/
/****�ﵽ��ʱ1΢�����µ�ʱ�䡣                                                   ****/
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

