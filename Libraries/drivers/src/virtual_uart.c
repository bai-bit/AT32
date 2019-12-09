
#include<virtual_uart.h>

void uart1_io_config(void)
{
	//使能GPIO口
	//使能AFIO时钟
	//配置AFIO,EXTIC的寄存器，外部中断输入源
	//初始化外部中断
	//使能外部中断
	GPIO_Init(HW_GPIOA, gpio_pin_9, gpio_speed_50MHz, mode_out_pp);
	
	GPIO_Init(HW_GPIOA, gpio_pin_10, gpio_speed_inno, mode_in_floating);
	AFIO_Init(HW_GPIOA, gpio_pin_10);
	exti_init(exti_line10, interrupt, failling, ENABLE);
	NVIC_Init(EXTI15_10_IRQn, 2, 3, ENABLE);
}


char uart_read_buf[UART_SIZE] = "";
uint16_t uart_read_status;
uint32_t uart_delayus;
uint32_t uart_delayus_again;
extern GPIO_Type* gpio_list[];


void uart_virtual(uint32_t baud)
{
	if(baud == BAUD_115200)
	{
		uart_delayus = VIRTUAL_BAUD  /  (baud - 4500) ;
		uart_delayus_again = uart_delayus / 2;
	}
	else
	{
		uart_delayus = VIRTUAL_BAUD  /  baud ;
		uart_delayus_again = uart_delayus / 2;
	}
	
	uart1_io_config();
}

uint8_t uart_recvive_data(uint32_t uartnum)
{
	//return uart_list[uartnum]->DT & UART_DATA_MASK;
	//采取模拟串口时序的方式来读取数据
	//PA9  发送引脚   writedata()
	//PA10 接受引脚   readdata()
	//该函数位接受函数，调用readdata()函数，调用delayus()函数
	//for循环
	int i;
	uint8_t data = 0;
	delayus(uart_delayus_again);
		
	for(i = 0;i < 8;i++)
	{
		//每读取到一位数值，将它放到数组中。
		//串口默认LSB发送（小端存储）
		//发送过来的数据是ASCII码格式
		delayus(uart_delayus - 1); 
		data |= (read_gpioport(HW_GPIOA, gpio_pin_10) << i);
	}
	
	return data;
}

void uartsend_data(uint32_t uartnum)
{
	//调用写入函数
	uint32_t i = 0;
	
	while(uart_read_buf[i] != '\0')
			virtual_serial(uartnum, uart_read_buf[i++]);
}


uint16_t  readdata(uint32_t uartnum)
{
	//读取PA10引脚
	switch(uartnum)
	{
		case HW_USART1:
			return read_gpioport(HW_GPIOA, gpio_pin_10);
		case HW_USART2:
		case HW_USART3:
		case HW_UART4:
		case HW_UART5:
			
			break;
	}
	return 0;
}

void virtual_serial(uint32_t uartnum, uint16_t data)
{
	//封装串口数据
	//模拟串口的时序
	uint16_t bit_data = 0, i = 0;
	
	data &= 0xff;
	data |= 1 << 8;
	data <<= 1;
	for(i = 0; i < 10; i++)
	{
		bit_data = (data >>i) & 0x01;
		GPIO_PinWrite(HW_GPIOA, gpio_pin_9, bit_data);
		delayus(uart_delayus - 1);
	}
}

void writedata(uint32_t uartnum, uint16_t data)
{
	//写入PA9引脚
	switch(uartnum)
	{
		case HW_USART1:
			GPIO_PinWrite(HW_GPIOA, gpio_pin_9, data);
			break;
		case HW_USART2:
		case HW_USART3:
		case HW_UART4:
		case HW_UART5:
			
			break;
	}
}


void EXTI15_10_IRQHandler(void)
{
	//判断中断线
	uint8_t ret;
	if(get_extiinter(exti_line10) == SET)
	{
		//读数据
		//判断发送结束标志
				 
		if((uart_read_status & UART_STATUS_NLINE) == RESET )
		{
			ret = uart_recvive_data(HW_USART1);
			if(uart_read_status & UART_STATUS_ENTER)
				if(ret == 0x0a)
					uart_read_status |= UART_STATUS_NLINE;
				else
					uart_read_status = RESET;
			else if(ret == 0x0d)
			{
				uart_read_status |= UART_STATUS_ENTER;
				uart_read_buf[uart_read_status & UART_STATUS_MASK] = '\0';
			}
			else
			{
				uart_read_buf[uart_read_status & UART_STATUS_MASK] = ret;
				uart_read_status++;
				if((uart_read_status & UART_STATUS_MASK) > UART_SIZE)
					uart_read_status = RESET;
			}
		}
		clean_extiinter(exti_line10);
	}
}
