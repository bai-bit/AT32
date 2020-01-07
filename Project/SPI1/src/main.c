#include "systemclk.h"
#include "spi.h"
#include "common.h"
#include "uart.h"
#include "gpio_init.h"

int main(int argc,const char *argv[])
{
    SysClk_PLLEN(PLLCLK_MUL_192MHz);
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    GPIO_Init(HW_GPIOA,GPIO_PIN_9,GPIO_Mode_AF_PP);
    GPIO_Init(HW_GPIOA,GPIO_PIN_10,GPIO_Mode_IN_FLOATING);
    UART_Init(HW_USART1,BAUD_115200);	
    DelayInit();
    SPI_Init(HW_SPI1, SPI_BAUDRATE_64);
    u8 ch;
    u8 data;
    char i = 0;
    
    while(1)
    {
        if(i > 127)
        {
            i=0;
            delayms(500);
        }
        data = '\0' + i++;
    
        ch = SPI_TransferData(HW_SPI1,data);
        printf("ch = [%c]\r\n",ch);
    }
}
