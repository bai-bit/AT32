#include<at32f4xx.h>
#include<uart.h>
#include<shell.h>

int main(int argc,const char *argv[])
{
	sysclk_PLLEN(PLLCLK_MUL_192MHz);
	led_red_init();
	DelayInit();
	uart1_init(115200);
	  
	while(1)
	{
//	if(strncmp(uart_rx_buf,"shell_start",11) == 0)
			shell_loop();
	}
}
