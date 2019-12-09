#include<uart.h>

USART_Type* uart_list[]={USART1, USART2, USART3, UART4, UART5};
char uart_rx_buf[UART_SIZE]= "";
uint16_t uart_rx_status;

void uart1_init(uint32_t baud)
{
	GPIO_Init(HW_GPIOA, gpio_pin_9, gpio_speed_50MHz, mode_af_pp);
	GPIO_Init(HW_GPIOA, gpio_pin_10, gpio_speed_inno, mode_in_floating);
	
	USART_Init(HW_USART1, baud, wordlength_8, stopbits_1, parity_none, mode_tx | mode_rx, hardcontrol_no);
	USART_ITConfig(HW_USART1, uart_int_rdne, ENABLE);
	USART_Cmd(HW_USART1, ENABLE);
	
	NVIC_Init(USART1_IRQn, 2, 2, ENABLE);
}


void USART_Init(uint32_t uartnum, uint32_t baud, uint32_t wordlength, uint32_t stopbits, uint32_t parity, uint32_t mode, uint32_t hardcontrol)
{
	//开启串口时钟
	//配置串口寄存器
	//CR1:停止位
	//CR2:字长，模式，奇偶校验
	//CR3:硬件流控制
	switch(uartnum)
	{
		case HW_USART1:
			RCC->APB2EN |= RCC_APB2EN_USART1EN;
			break;
		case HW_USART2:
			RCC->APB1EN |= RCC_APB1EN_USART2EN;
			break;
		case HW_USART3:
			RCC->APB1EN |= RCC_APB1EN_USART3EN;
			break;
		case HW_UART4:
			RCC->APB1EN |= RCC_APB1EN_UART4EN;
			break;
		case HW_UART5:
			RCC->APB1EN |= RCC_APB1EN_UART5EN;
			break;
		default:
			break;
	}
	
	uint32_t tmpreg;

	tmpreg = uart_list[uartnum]->CTRL1;
  tmpreg &= CTRL1_MASK;
  tmpreg |= wordlength | parity | mode;
  
  uart_list[uartnum]->CTRL1 = (uint16_t)tmpreg;
	
  tmpreg = uart_list[uartnum]->CTRL2;
	tmpreg &= CTRL2_STOP_Mask;
	tmpreg |= stopbits;

  uart_list[uartnum]->CTRL2 = (uint16_t)tmpreg;

  tmpreg = uart_list[uartnum]->CTRL3;
  tmpreg &= CTRL3_Mask;
  tmpreg |= hardcontrol;
  uart_list[uartnum]->CTRL3 = (uint16_t)tmpreg;

	lib_set_brr(uartnum,baud);
}


void lib_set_brr(uint32_t uartnum, uint32_t baud)
{
	uint32_t apbclock;
	uint32_t tmpreg,integerdivider,fractionaldivider;
	
	
	if(uartnum == HW_USART1)
		apbclock = getclock_frequency(pclk1);
	else
		apbclock = getclock_frequency(pclk2);
  
	integerdivider = ((25 * apbclock) / (4 * baud));
  
  tmpreg = (integerdivider / 100) << 4;

  fractionaldivider = integerdivider - (100 * (tmpreg >> 4));

	tmpreg |= ((((fractionaldivider * 16) + 50) / 100)) & ((uint8_t)0x0F);
 
	uart_list[uartnum]->BAUDR = (uint16_t)tmpreg;
}

void USART_ITConfig(uint32_t uartnum, uint16_t uart_interrupt, FunctionalState NewStatus)
{
	//开启相应的中断
	//首先获取串口的基地址
	//然后把中断值屏蔽高八位，将低八位右移五位，确定配置的寄存器,
	//接下来获取第五位的值，确定配置的位
	//然后通过基地址加偏移地址，确定到寄存器的地址，根据NewState，配置某一个位
	uint32_t uartxbase = 0;
	uint16_t tempreg = 0, setmov = 0, configbit = 0;
	
	uartxbase = (uint32_t)uart_list[uartnum];
	
	tempreg = (uart_interrupt & 0xff) >> 5;
	
	setmov = uart_interrupt & REGMASK;
	
	configbit = 1 << setmov;
	
	if(0x01 == tempreg)
		uartxbase += 0x0c;
	else if(tempreg == 0x02)
		uartxbase += 0x10;
	else
		uartxbase += 0x14;
	
	if(NewStatus == ENABLE)
		*(uint32_t *)uartxbase |= configbit;
	else
		*(uint32_t *)uartxbase &= ~configbit;
}


void USART_Cmd(uint32_t uartnum, FunctionalState NewStatus)
{
	//使能串口,配置CR1寄存器的第十三位
	
	if(NewStatus == ENABLE)
		uart_list[uartnum]->CTRL1 |= UART_ENABLE;
	else
		uart_list[uartnum]->CTRL1 &= ~UART_ENABLE;
}


void log_uart(uint8_t uartnum, char *buf)
{
	u8 send_buf[UART_SIZE];
	uint32_t i;
	for(i=0;i<UART_SIZE;i++)
	{
		send_buf[i] = *buf++;
		if(send_buf[i] == '\0')
			i=UART_SIZE;
	}
	for(i=0;i<UART_SIZE;i++)
	{
		call_back_send(uartnum,send_buf[i]);
		if(send_buf[i] == '\0')
			i = UART_SIZE;
	}
}

void call_back_send(uint8_t uartnum, char ch)
{
	while((uart_list[uartnum]->STS &0x80) == 0)
		continue;
	uart_list[uartnum]->DT =(uint8_t) ch;
}


ITStatus uart_getinter(uint32_t uartnum, uint32_t uart_interrupt)
{
	//首先确定寄存器
	//然后确定配置的中断
	//接下来确定相应中断的标志位
	//通过if。。。else语句返回状态
	uint16_t uartreg = 0, interbit = 0, uartinter = 0, uartflag = 0, flagmask = 0;
	
	uartreg = (uint8_t)uart_interrupt >> 0x05;
	interbit = uart_interrupt & REGMASK;
	interbit = 0x01 << interbit;
	
	if(0x01 == uartreg)
		uartinter = uart_list[uartnum]->CTRL1 & interbit;
	else if(0x02 == uartreg)
		uartinter = uart_list[uartnum]->CTRL2 & interbit;
	else
		uartinter = uart_list[uartnum]->CTRL3 & interbit;
	
	uartflag = uart_interrupt >> 0x08;
	flagmask = 0x01 << uartflag;
	flagmask &= uart_list[uartnum]->STS;
	
	if(uartinter != 0 && flagmask != 0)
		return SET;
	else
		return RESET;
}
	
uint16_t uartrecvive_data(uint32_t uartnum)
{
	return uart_list[uartnum]->DT & UART_DATA_MASK;
}


static u8 rev1;
static u8 flag = 0;
void USART1_IRQHandler(void)
{
	//接受数据
	//判断接受标志位
	//判断接受的数据，0x0a 0x0d	
	u8 rev = 0;
	
	if(uart_getinter(HW_USART1, uart_int_rdne) == SET)
	{
		rev = uartrecvive_data(HW_USART1);
		rev1 = rev;
		flag = 1;
		
		if((uart_rx_status & UART_STATUS_NLINE) == RESET)
		{
			if(uart_rx_status & UART_STATUS_ENTER)
			{
				if(rev == 0x0a)
					uart_rx_status |= UART_STATUS_NLINE;
				else
					uart_rx_status = RESET;
			}
			else
			{
				if(rev == 0x0d)
					uart_rx_status |= UART_STATUS_ENTER;
				else
				{
					uart_rx_buf[uart_rx_status & UART_STATUS_MASK] = rev;
					uart_rx_status ++;
					if(uart_rx_status > UART_SIZE)
						uart_rx_status = RESET;
				}
			}
		}
	}
}


#pragma import(__use_no_semihosting)             
                 
struct __FILE 
{ 
	int handle; 

}; 

FILE __stdout;
FILE __stdin;
FILE __stderr;
   
void _sys_exit(int x) 
{ 
	x = x; 
} 

//void _ttywrch(int ch)
//{
//	ch = ch;
//}

int fputc(int ch, FILE *f)
{      
	while((USART1->STS & 0X40) == 0)
		continue;  
    USART1->DT = (u8) ch;      
	return ch;
}


int fgetc(FILE *stream)
{
	while(flag == 0)
		continue;
	flag = 0;
	
	return rev1;	
}

