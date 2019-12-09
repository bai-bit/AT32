# AUTO BAUD

这个历程是一个利用GPIO的中断来计算串口的波特率，实现自动匹配串口的baud。

思路：

​	1、串口的baud的倒数，就是串口数据的一个脉冲宽度。

​	2、由于数据的不同的，波形也就不一样，所以不能通过捕捉一个脉冲宽度来计算band的值。

​	3、观察串口的数据波形，总是在下降沿开始发送数据，最后一个上升沿结束发送。中间传送了多少位数据，不知道。这个需要去选择。

​	4、串口的协议：一个起始位，七位或八位的数据位，一位校验位，一位停止位。

​	5、所以一条完整的串口信号，分为10位，或者是11位。	

​	6、使能GPIO口的下降沿和上升沿触发中断。

​	7、下降沿触发中断，读取数值，上升沿触发中断，读取数值。他们的差值，就是一个时间值，但是这个时间值包含了几个脉冲，不知道。

​	8、systick的current value register，读取数值。



​	应该是开启下降沿触发，外部中断的优先级要和串口中断一样。

​	如果外部中断的优先级高于串口中断，会造成读取的数据不准，而且读取的时间也很乱。

​	从早上到现在，快六点了，我用xcom向开发板的PA10发送0x5a 0xa6，通过程序计算，只能算出一个近视值。

​	由于各种误差，无法精准的计算出baud的值。只能在程序中计算出一个脉冲宽度时间时，使用switch来选择一个固定的值。

​	过程中，我遇到一个困惑，就是串口中断的优先级和GPIO口的优先级。他们两个同时发生怎么办，肯定会影响到我外部中断处理函数的工作。所以在这一块纠结了很大一部分时间。

​	后来，通过测试和使用逻辑分析仪观测，和过程中捕捉到的时间。我突然想起来串口接收中断标志为的一个描述，当移位寄存器当中的数据被转移到USART->DT寄存器中，该标志位被置一。也就是说，当这个为置一时，才会触发中断，数据先经过GPIO口，才会进入到移位寄存器。等到八个数据位都到移位寄存器中时，才会统一转移到数据寄存器。

​	所以，这么看来，GPIO口的边沿触发中断时要先于串口中断触发。第一个字节过去之后，之后的字节就不敢保证了，肯定会造成一个工作，另一个等待的情况。后续的时间取值肯定是不准确的。等待一次数据发送完成后，下一次开始新一轮的发送，我们就可以继续取值。每次只取前七次的值。

​	测试过程中，为了方便快捷，我发送的是固定的数据。上面有提到。但是在实际过程中，取得前七次值中，第一个值特别的大。为了保证数据的可靠，打印了五分中的数据，第一组的差值都是非常大的，抛弃他，肯定不是我发送数据的头一个字节的时间点。

​	通过逻辑分析仪和打印的时间差值，第二个到第七个是我需要的数值，那好，我把他们加起来，除以9，就是一个脉冲宽度的持续时间。

​	等等，为什么是除以九，因为最后一个位是停止位，一般是高电平，这个高电平会一直持续到下一个数据得到来，所以这个位得时间点，我们不要他，因为他的时间我们无法准确捕捉。

​	这样计算出来的一个脉冲宽度时间点是一个近似值，并不准确。所以用这个近似值再去计算baud的值，那么baud的值和标准的波特率比较，误差会更大。

​	所以，在这里，我用一个范围来确定一个固定波特率。下面贴出我的代码，在代码中再细说：

```c
void EXTI15_10_IRQHandler(void)
{
	if(get_extiinter(exti_line10))
	{
		time = SysTick->VAL & SysTick_VAL_CURRENT_Msk;
//		clean_extiinter(exti_line10);    //清除中断挂起位
		EXTI->PND |= 2 << exti_line10;
    }
		tatol[i++] = time;
	//上面这几个步骤，应该尽可能的减少运算。代码越精简越好，所以在清除中断挂起位的时候，建议直接操作寄存器，
    //不使用函数调用。但是这一步：if(get_extiinter(exti_line10))，是必须要加的，因为这个中断处理函数
    //要集中处理6条中断线上的中断触发，这么多中断，就必须要区分开，每一条中断线对应的情况也有可能不同。
    //因为这一段代码是获取中断触发时的SYSTICK->VAL中数值，为了保证及时响应下一个边沿触发，所以它要尽可能的精简。
    //同时，这里也不能调用其他的中断。太耗时间。比如，在调试的时候，使用printf函数。
	if(i == SIZE)
	{
		for(int j = 0;j < SIZE;j++)
//		printf("tatol[%d] = [%f]	",j,tatol[j]);

		for(int m = 0;m <SIZE-1;m++)
		{
			dif_val = tatol[m] - tatol[m + 1];
			if(dif_val > 0)                   //这一步，是为了防止算出负数。
				temp[m] = dif_val;
//			printf("temp[%d] = [%f]	",m,temp[m]);
		}
		
		value = (temp[1] + temp[2] + temp[3] + temp[4] + temp[5] + temp[6] + temp[7])/BYTE_Mask;
		
//		printf("value = [%f]	",value);
//		printf("baud = [%d]\r\n",(int)(1000000.00/(value/24)));
		if(value >= 91 && value <= 95)//256000  93.75
			auto_baud = BAUD_256000;
		else if(value >= 52 && value <= 55)//460800 52.08
			auto_baud = BAUD_460800;
		else if(value >= 205 && value <= 210)//115200 208.33
			auto_baud = BAUD_115200;
		else if(value >= 2495 && value <= 2505)//9600 2500
			auto_baud = BAUD_9600;
		uart1_init(auto_baud);
		exti_init(exti_line10,interrupt,failling_and_rising,DISABLE);
		//上面这一步，是关闭PA10线上的边沿触发中断。
        //在这个中断处理函数中，他要处理10号线到15号线上的中断。所以不能关掉中断处理函数，只能关中断线
        //而且在中断处理函数中，关闭它自己，会造成程序跑飞。
        //printf("auto_baud = [%d]	",auto_baud);
		i=0;  //这个步骤，必须的加上。数组不能越界，越界会发生不可预知的错误。
              //比如，越界的部分，覆盖住其他的代码。如果万幸没有覆盖其他代码，i的值在自增到255后，下一次自增会变成0.
	}
}
```

​	这个函数所用到的一些变量，都是全局变量，要么是被static修饰的局部作用域变量。在程序第一次运行时可能取到的第一个计数值可能不够精准。但是之后的计数值的误差会小点。

```c\
double time;
double tatol[SIZE];
u8 i;
double dif_val = 0;
double temp[SIZE] = {0};
double value = 0;
uint32_t auto_baud;
```

​	在使用这个功能之前，首先要初始化各种资源。下面是代码，

```c
void auto_uart1_baud(void)
{
	GPIO_Init(HW_GPIOA, gpio_pin_10, gpio_speed_inno, mode_in_floating);
	AFIO_Init(HW_GPIOA,gpio_pin_10);
	exti_init(exti_line10,interrupt,failling_and_rising,ENABLE);
	NVIC_Init(EXTI15_10_IRQn,3,2,ENABLE);
	SysTick->LOAD = RELOAD_VALUE;
	SysTick->CTRL |=  SysTick_CTRL_ENABLE_Msk;
}
```

到我们的main函数中就更简单了。

```c
int main(int argc,const char *argv[])
{
	sysclk_PLLEN(PLLCLK_MUL_192MHz);
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
//	uart1_init(BAUD_115200);
	DelayInit();
	
	auto_uart1_baud();

	for(;;)
		printf("welcome to xcom");
	
}
```

​                                                                                                               2019年11月2日         15:46