#include<auto_uart_baud.h>

double time;
double tatol[SIZE];
u8 i;
double dif_val = 0;
double temp[SIZE] = {0};
double value = 0;
uint32_t auto_baud;

void auto_uart1_baud(void)
{
	GPIO_Init(HW_GPIOA, gpio_pin_10, gpio_speed_inno, mode_in_floating);
	AFIO_Init(HW_GPIOA,gpio_pin_10);
	exti_init(exti_line10,interrupt,failling_and_rising,ENABLE);
	NVIC_Init(EXTI15_10_IRQn,3,2,ENABLE);
	SysTick->LOAD = RELOAD_VALUE;
	SysTick->CTRL |=  SysTick_CTRL_ENABLE_Msk;
}

void EXTI15_10_IRQHandler(void)
{
	if(get_extiinter(exti_line10))
	{
		time = SysTick->VAL & SysTick_VAL_CURRENT_Msk;
		clean_extiinter(exti_line10);
	}
		tatol[i++] = time;
	
	if(i == SIZE)
	{
		for(int j = 0;j < SIZE;j++)
//	printf("tatol[%d] = [%f]	",j,tatol[j]);

		for(int m = 0;m <SIZE-1;m++)
		{
			dif_val = tatol[m] - tatol[m + 1];
			if(dif_val > 0)
				temp[m] = dif_val;
//		printf("temp[%d] = [%f]	",m,temp[m]);
		}
		
		value = (temp[0] + temp[1] + temp[2] + temp[3] + temp[4] + temp[5] + temp[6])/BYTE_Mask;
		
//	printf("value = [%f]	",value);
//	printf("baud = [%d]\r\n",(int)(1000000.00/(value/24)));
		if(value >= 91 && value <= 95)//256000  93.75
			auto_baud = BAUD_256000;
		else if(value >= 52 && value <= 55)//460800 52.08
			auto_baud = BAUD_460800;
		else if(value >= 205 && value <= 210)//115200 208.33
			auto_baud = BAUD_115200;
		else if(value >= 2480 && value <= 2515)//9600 2500
			auto_baud = BAUD_9600;
		uart1_init(auto_baud);
		exti_init(exti_line10,interrupt,failling_and_rising,DISABLE);
		
//	printf("auto_baud = [%d]	",auto_baud);
		i=0;
	}
}
