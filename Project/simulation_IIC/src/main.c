#include<AT24C02.h>
#include<systemclk.h>
#include<systick.h>

int main(int argc,const char *argv[])
{
	sysclk_PLLEN(PLLCLK_MUL_192MHz);
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	DelayInit();
	uart1_init(BAUD_115200);
	
	AT24C02_Init();
	
}
