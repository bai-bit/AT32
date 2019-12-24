#include<flash.h>
#include<systemclk.h>
#include<nvic.h>
#include<systick.h>
#include<uart.h>
#include<string.h>

#define FLASH_TES_ADDR	(0x08000000 + 200*1024)
int main(int argc,const char *argv[])
{
    int i;
    
    char buf[10] = "123456";
    char rbuf[10] = "1";
    sysclk_PLLEN(PLLCLK_MUL_192MHz);
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

	DelayInit();
    uart1_init(BAUD_115200);
    
    flash_init();
    printf("flash test\r\n");

	Flash_ErasePage(FLASH_TES_ADDR);//擦除这个扇区
	Flash_WritePageWord(FLASH_TES_ADDR,(uint16_t *)buf,8);//写入整个扇区
	flash_read(FLASH_TES_ADDR,(uint16_t *)rbuf,8);

    printf("2\r\n");
    printf("flash test\r\n");
 
	for(i = 0;i<10;i++)
		printf("%d  ",rbuf[i]);
	while(1);

	
}
