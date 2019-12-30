#include<uart.h>
#include<strfunc.h>
#include<systick.h>


void SysTick_Handler(void);
void Reset_Handler(void);
int main(int argc, const char *argv[])
{
	sysclk_PLLEN(PLLCLK_MUL_192MHz);
	DelayInit();
	uart1_init(115200);
	systick_setexception(true);
	SysTick->LOAD = (float)getclock_frequency(pll) / 80000;
	
	
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
