#include<systemclk.h>
#include<spi.h>
#include<systick.h>
#include<led.h>

int main(int argc,const char *argv[])
{
	sysclk_PLLEN(PLLCLK_MUL_192MHz);
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	uart1_init(BAUD_115200);	
    DelayInit();
	spi1_init();
	u8 ch;
	u8 data;
	char i = 0;
    printf("123\t\n");
	while(1)
	{
		if(i > 127)
		{
            i=0;
            delayms(500);
        }
		data = '\0' + i++;
//		while(spi_GetFlagStatus(SPI1,SPI_Flag_TX) == RESET);
//		GPIO_PinWrite(HW_GPIOA,gpio_pin_4,0);
//       // printf("1\t\n");
//		SPI1->DT = data;
//		while(spi_GetFlagStatus(SPI1,SPI_Flag_BSY) == SET);
//		GPIO_PinWrite(HW_GPIOA,gpio_pin_4,1);
//		ch = SPI1->DT;
//       // printf("2\t\n");
//		//if(ch != data)
        ch = spi_RWdata(SPI1,data);
			printf("ch = [%c]\r\n",ch);
        
	}
}
