#include<EN25QXXX.h>
#include<systemclk.h>
#include<uart.h>
#include<string.h>
#define SIZE 12

int main(int argc,const char *argv[])
{
	u8 buf[SIZE] = "hello world";
	u8 rbuf[SIZE] = "";
	u8 status = 0;
	int i;
    sysclk_PLLEN(PLLCLK_MUL_192MHz);
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    DelayInit();
    uart1_init(BAUD_115200);
    EN25QXXX_init(SPI1);
   

	status = EN25QXXX_read_register();
	printf("status = [%X]\t\n",status);
	EN25QXXX_erase_sector(0x800000);
//	EN25QXXX_write_nocheck(0x800000,buf,SIZE);
	EN25QXXX_write_data(0x800000,buf,SIZE);
	EN25QXXX_read_data(0x800000,rbuf,SIZE);
	//fputs((const char *)rbuf,stdout);
	printf("rbuf = [%s]\t\n",rbuf);
//	for(i = 0;i < SIZE;i++)
//		printf("buf = [ %X ]   ",buf[i]);
//	for(i = 0;i < 2 * SIZE;i++)
//		printf("rbuf = [ %X ]  ",rbuf[i]);
    while(1)
    {
        
      
    }
        
}
