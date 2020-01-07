//屏蔽systeminit函数
//然后在main函数中实现systeminit函数的功能
//先使用HSI,然后HSE,最后PLL
#include "systemclk.h"
#include "gpio_init.h"
#include "common.h"
#include "led.h"

int main(int argc,const char *argv[])
{
	System_Init();
	
	DelayInit();
	led_red_init();
	
	while(1)
	{
		GPIO_PinToggle(HW_GPIOD,GPIO_PIN_13);
		delayms(500);
	}
}
