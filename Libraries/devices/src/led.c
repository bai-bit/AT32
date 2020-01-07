//led function
#include<led.h>

void led_red_init(void)
{
	GPIO_Init(HW_GPIOD, GPIO_PIN_13, GPIO_Mode_Out_PP);
	GPIO_PinWrite(HW_GPIOD, GPIO_PIN_13, RESET);
}
