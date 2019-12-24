#include<EN25QXXX.h>
#include<systemclk.h>
#include<uart.h>
#include<string.h>
#include<math.h>
#include<strfunc.h>
#include<spi_flash.h>

#define SIZE 64
#define TEST_ADDR 0x321654

spi_flash_dev_t operation = {
	.open = EN25QXXX_init,
	.read = EN25QXXX_read,
	.write = EN25QXXX_write_data,
	.clear = EN25QXXX_clear,
	.release = EN25QXXX_close
};

int main(int argc,const char *argv[])
{
    sysclk_PLLEN(PLLCLK_MUL_192MHz);
	uint8_t buf[SIZE] = "hello world,welcome to nowyork";  
	uint8_t rbuf[SIZE] = "";
	uint16_t i = 0;

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	
    DelayInit();
    uart1_init(BAUD_115200);
	
	GPIO_Init(HW_GPIOA, gpio_pin_5, gpio_speed_50MHz, mode_af_pp);
	GPIO_Init(HW_GPIOA, gpio_pin_7, gpio_speed_50MHz, mode_af_pp);
	GPIO_Init(HW_GPIOA, gpio_pin_6, gpio_speed_inno,mode_in_floating);
	
	i = spi_flash_open(SPI1,&operation);
	printf("EN25Q ID = [%X]\r\n",i);
	
	spi_flash_read(SPI1,TEST_ADDR,rbuf,SIZE,DATA);

	spi_flash_write(SPI1,TEST_ADDR,buf,sizeof(buf),DATA);

	spi_flash_read(SPI1,TEST_ADDR,rbuf,SIZE,DATA);

	printf("rbuf3 = [%s]  \r\n",rbuf);
	
	printf("test is over\r\n");


    while(1)
    {
        
      
    }
        
}
