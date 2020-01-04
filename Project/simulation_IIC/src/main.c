#include<AT24C02.h>
#include<systemclk.h>
#include<common.h>
#include<uart.h>
#include<gpio_init.h>
#define SIZE 64

int main(int argc,const char *argv[])
{
    sysclk_PLLEN(PLLCLK_MUL_192MHz);
    uint8_t buf[SIZE] = "ladies and gentlemen,good everyone";
    uint8_t rbuf[SIZE] = {};
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    DelayInit();
    GPIO_Init(HW_GPIOA, GPIO_PIN_9,GPIO_Mode_AF_PP);
    GPIO_Init(HW_GPIOA, GPIO_PIN_10,GPIO_Mode_IN_FLOATING);
    UART_Init(HW_USART1,BAUD_115200);
    
    AT24CXX_Init();
    
    if(AT24CXX_check())
    {
        printf("AT24C02 is OK\r\n");
    }
    AT24CXX_write(253,buf,sizeof(buf));
    AT24CXX_read(253,rbuf,SIZE);
    printf("rbuf = [%s]\r\n",rbuf);
    while(1)
        continue;
	
}
