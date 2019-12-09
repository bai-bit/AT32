//屏蔽systeminit函数
//然后在main函数中实现systeminit函数的功能
//先使用HSI,然后HSE,最后PLL
#include<systemclk.h>
#include<gpio_init.h>
#include<systick.h>
#include<led.h>
extern uint32_t SystemCoreClock1;


int main(int argc,const char *argv[])
{
	sysclk_PLLEN(PLLCLK_MUL_192MHz);
	
	DelayInit();
	led_red_init();
	CLKOUT_Init(HW_GPIOA,gpio_pin_8);
	
	while(1)
	{
		GPIO_PinToggle(HW_GPIOD,gpio_pin_13);
		delayms(500);
	}
}
