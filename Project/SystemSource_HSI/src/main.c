//����systeminit����
//Ȼ����main������ʵ��systeminit�����Ĺ���
//��ʹ��HSI,Ȼ��HSE,���PLL
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
