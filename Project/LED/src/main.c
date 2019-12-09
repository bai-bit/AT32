#include<led.h>
#include<systick.h>

int main(int argc, const char *argv[])
{
	sysclk_PLLEN(PLLCLK_MUL_192MHz);
	DelayInit();
	led_red_init();
	
	while(1)
	{
		GPIO_PinToggle(HW_GPIOD,gpio_pin_13);
		delayms(500);
	}
}
