//����systeminit����
//Ȼ����main������ʵ��systeminit�����Ĺ���
//��ʹ��HSI,Ȼ��HSE,���PLL
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
