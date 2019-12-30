#include<AT24C02.h>
#include<systemclk.h>
#include<systick.h>
#include<uart.h>

int main(int argc,const char *argv[])
{
	sysclk_PLLEN(PLLCLK_MUL_192MHz);
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	DelayInit();
    GPIO_Init(HW_GPIOA, GPIO_PIN_9, GPIO_Speed_50MHz, GPIO_Mode_AF_PP);
	GPIO_Init(HW_GPIOA, GPIO_PIN_10, GPIO_Speed_INNO, GPIO_Mode_IN_FLOATING);
	UART_Init(HW_USART1,BAUD_115200);
	printf("1");
	AT24CXX_Init();
    printf("2");
    if(AT24CXX_check())
    {
        printf("123");
    }
    printf("7");
    while(1)
        continue;
	
}
