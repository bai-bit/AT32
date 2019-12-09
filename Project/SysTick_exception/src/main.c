
#include<uart.h>
#include<strfunc.h>


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
	//printf("count = [%d] ",123);
		delayms(500);
	}
}



void SysTick_Handler(void)
{
	//��ӡ����ֵ
	char *str = 0;
	static uint32_t count = 0;
	count++;
//	str = itoa(count);
//	log_uart(HW_USART1, str);
	printf("count = [%d] ",count);
}
