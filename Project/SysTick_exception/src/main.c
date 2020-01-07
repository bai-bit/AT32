#include "uart.h"
#include "strfunc.h"
#include "common.h"
#include "systemclk.h"
#include "gpio_init.h"

void SysTick_Handler(void);
void Reset_Handler(void);
int main(int argc, const char *argv[])
{
	SysClk_PLLEN(PLLCLK_MUL_192MHz);
	DelayInit();
    GPIO_Init(HW_GPIOA, GPIO_PIN_9, GPIO_Mode_AF_PP);
    GPIO_Init(HW_GPIOA, GPIO_PIN_10, GPIO_Mode_IN_FLOATING);
	UART_Init(HW_USART1, BAUD_115200);
	systick_setexception(true);
	SysTick->LOAD = (float)GetClock_Frequency(pll) / 80000;
	
	
	printf("0x%p\r\n", SysTick_Handler);
	while(1)
	{
		delayms(500);
	}
}



void SysTick_Handler(void)
{
	//打印计数值

	static uint32_t count = 0;
	count++;

	printf("count = [%d] ",count);
}
