//屏蔽systeminit函数
//然后在main函数中实现systeminit函数的功能
//先使用HSI,然后HSE,最后PLL
#include<systemclk.h>
#include<gpio_init.h>
#include<systick.h>
#include<led.h>

int main(int argc,const char *argv[])
{
	system_init();
	
	DelayInit();
	led_red_init();
	
	while(1)
	{
		GPIO_PinToggle(HW_GPIOD,gpio_pin_13);
		delayms(500);
	}
}
