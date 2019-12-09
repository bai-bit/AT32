#include<systemclk.h>
#include<uart.h>
#include<auto_uart_baud.h>



int main(int argc,const char *argv[])
{
	//初始化时钟系统
	//进行中断优先级分组
	//使能串口，但是不使能对应的GPIO口
	//将对应的GPIO口初始化为输入，双边沿触发
	//使能systick
	//在GPIO口中断函数中计算上升沿和下降沿之间的时间，将这个时间保存到一个数组中。
	//查找二十组，然后计算平均值。
	//然后用1000000除以这个平均值，得到的就是近似于baud的值，
	//这个值不是很准确，需要进一步选择
	sysclk_PLLEN(PLLCLK_MUL_192MHz);
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
//uart1_init(115200);
	DelayInit();
	
	auto_uart1_baud();

	for(;;)
		printf("welcome to xcom");
	
}
 


