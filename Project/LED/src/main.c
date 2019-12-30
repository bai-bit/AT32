#include<led.h>
#include<systick.h>
#include<systemclk.h>

int main(int argc, const char *argv[])
{
	sysclk_PLLEN(PLLCLK_MUL_192MHz);
	DelayInit();
	led_red_init();
	
	while(1)
	{
//		GPIO_PinToggle(HW_GPIOD,GPIO_PIN_13);
        PDout(13) = 1;
		delayms(500);
        PDout(13) = 0;
        delayms(500);
	}
}
