//led function
#include<led.h>


void led_red_init(void)
{
	GPIO_Init(HW_GPIOD, gpio_pin_13, gpio_speed_50MHz, mode_out_pp);
	GPIO_PinWrite(HW_GPIOD, gpio_pin_13, RESET);
}

