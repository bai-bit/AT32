#include "uart.h"

USART_Type* uart_list[]={USART1, USART2, USART3, UART4, UART5};
char uart_rx_buf[UART_RX_BUF_SIZE]= "";
uint16_t uart_rx_status;

uint32_t UART_Init(uint32_t instance, uint32_t baudrate)
{
    //开启串口时钟
    //配置串口寄存器
    //CR1:停止位
    //CR2:字长，模式，奇偶校验
    //CR3:硬件流控制
    switch (instance)
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
            return 0;
    }
	
    uint32_t tmpreg = 0;

    tmpreg = uart_list[instance]->CTRL1;
    tmpreg &= CTRL1_MASK;
    tmpreg |= USART_CTRL1_REN | USART_CTRL1_TEN;
    uart_list[instance]->CTRL1 = (uint16_t)tmpreg;
    
    tmpreg = uart_list[instance]->CTRL2;
    tmpreg &= CTRL2_STOP_Mask;


    uart_list[instance]->CTRL2 = (uint16_t)tmpreg;

    tmpreg = uart_list[instance]->CTRL3;
    tmpreg &= CTRL3_Mask;
   
    uart_list[instance]->CTRL3 = (uint16_t)tmpreg;

    UART_SetBaudRate(instance,baudrate);
    
    USART_ITConfig(HW_USART1, UART_INT_RDNE, ENABLE);
    USART_Cmd(HW_USART1, ENABLE);
    
    NVIC_Init(USART1_IRQn, 2, 2, ENABLE);
    
    return 1;
}

uint32_t UART_DeInit(uint32_t instance)
{
    uart_list[instance]->CTRL1 &= ~USART_CTRL1_UEN;
    return 0;
}
void UART_SetBaudRate(uint32_t instance, uint32_t baud)
{
    uint32_t apbclock = 0;
    uint32_t tmpreg = 0, integerdivider = 0, fractionaldivider = 0;

    if (instance == HW_USART1)
        apbclock = GetClock_Frequency(pclk1);
    else
        apbclock = GetClock_Frequency(pclk2);
    
    integerdivider = ((25 * apbclock) / (4 * baud));
    
    tmpreg = (integerdivider / 100) << 4;
    
    fractionaldivider = integerdivider - (100 * (tmpreg >> 4));
    
    tmpreg |= ((((fractionaldivider * 16) + 50) / 100)) & ((uint8_t)0x0F);
    
    uart_list[instance]->BAUDR = (uint16_t)tmpreg;
}

void USART_ITConfig(uint32_t instance, uint16_t uart_interrupt, FunctionalState NewStatus)
{
    //开启相应的中断
    //首先获取串口的基地址
    //然后把中断值屏蔽高八位，将低八位右移五位，确定配置的寄存器,
    //接下来获取第五位的值，确定配置的位
    //然后通过基地址加偏移地址，确定到寄存器的地址，根据NewState，配置某一个位
    uint32_t uartxbase = 0;
    uint16_t tempreg = 0, setmov = 0, configbit = 0;
    
    uartxbase = (uint32_t)uart_list[instance];
    
    tempreg = (uart_interrupt & 0xff) >> 5;
    
    setmov = uart_interrupt & REGMASK;
    
    configbit = 1 << setmov;
    
    if (0x01 == tempreg)
        uartxbase += 0x0c;
    else if (tempreg == 0x02)
        uartxbase += 0x10;
    else
        uartxbase += 0x14;
	
    if (NewStatus == ENABLE)
        *(uint32_t *)uartxbase |= configbit;
    else
        *(uint32_t *)uartxbase &= ~configbit;
}


void USART_Cmd(uint32_t instance, FunctionalState NewStatus)
{
    //使能串口,配置CR1寄存器的第十三位
    
    if (NewStatus == ENABLE)
        uart_list[instance]->CTRL1 |= USART_CTRL1_UEN;
    else
        uart_list[instance]->CTRL1 &= ~USART_CTRL1_UEN;
}




ITStatus UART_GetInter(uint32_t instance, uint32_t uart_interrupt)
{
    //首先确定寄存器
    //然后确定配置的中断
    //接下来确定相应中断的标志位
    //通过if。。。else语句返回状态
    uint16_t uartreg = 0, interbit = 0, uartinter = 0, uartflag = 0, flagmask = 0;
    
    uartreg = (uint8_t)uart_interrupt >> 0x05;
    interbit = uart_interrupt & REGMASK;
    interbit = 0x01 << interbit;
    
    if (0x01 == uartreg)
        uartinter = uart_list[instance]->CTRL1 & interbit;
    else if (0x02 == uartreg)
        uartinter = uart_list[instance]->CTRL2 & interbit;
    else
        uartinter = uart_list[instance]->CTRL3 & interbit;
	
    uartflag = uart_interrupt >> 0x08;
    flagmask = 0x01 << uartflag;
    flagmask &= uart_list[instance]->STS;
    
    if (uartinter && flagmask)
        return SET;
    else
        return RESET;
}
	
uint16_t UART_RecviveData(uint32_t instance)
{
    return uart_list[instance]->DT & UART_DATA_MASK;
}

uint32_t UART_GetChar(uint32_t instance, uint8_t *ch)
{
    if (uart_list[instance]->STS & (1 << 5))
    {
        *ch = (uart_list[instance]->DT & 0xFF);
        return 0;
    }
    return 1;
}
void UART_PutChar(uint32_t instance, uint8_t ch)
{
    while (!((uart_list[instance]->STS) & (1 << 7))); 
    uart_list[instance]->DT  = ch;             
}

uint32_t UART_SetIntMode(uint32_t instance, UART_Int_t mode, uint8_t val)
{
    if(mode == kUART_IntTx)
        USART_ITConfig(instance, UART_INT_TXE, val);
    else if(mode == kUART_IntRx)
        USART_ITConfig(instance, UART_INT_RDNE, val);
    return 0;
}

static uint8_t rev1;
static uint8_t flag = 0;
void USART1_IRQHandler(void)
{
    //接受数据
    //判断接受标志位
    //判断接受的数据，0x0a 0x0d	
    uint8_t rev = 0;
    
    if (UART_GetInter(HW_USART1, UART_INT_RDNE) == SET)
    {
        rev = UART_RecviveData(HW_USART1);
        rev1 = rev;
        flag = 1;
    
        if ((uart_rx_status & UART_STATUS_NLINE) == RESET)
        {
            if (uart_rx_status & UART_STATUS_ENTER)
            {
                if (rev == 0x0a)
                    uart_rx_status |= UART_STATUS_NLINE;
                else
                    uart_rx_status = RESET;
            }
            else
            {
                if (rev == 0x0d)
                    uart_rx_status |= UART_STATUS_ENTER;
                else
                {
                    uart_rx_buf[uart_rx_status & UART_STATUS_MASK] = rev;
                    uart_rx_status ++;
                    if(uart_rx_status > UART_RX_BUF_SIZE)
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

int fputc(int ch, FILE *f)
{      
    while((USART1->STS & 0X40) == 0)
        continue;  
    USART1->DT = (uint8_t) ch;      
    return ch;
}

int fgetc(FILE *stream)
{
    while(flag == 0)
        continue;
    flag = 0;
    
    return rev1;	
}

void log_uart(uint8_t instance, char *buf)
{
    uint8_t send_buf[UART_RX_BUF_SIZE] = "";
    uint32_t i = 0;
    for (i = 0; i < UART_RX_BUF_SIZE; i++)
    {
        send_buf[i] = *buf++;
        if (send_buf[i] == '\0')
            i = UART_RX_BUF_SIZE;
    }
    for (i = 0; i < UART_RX_BUF_SIZE; i++)
    {
        call_back_send(instance,send_buf[i]);
        if (send_buf[i] == '\0')
            i = UART_RX_BUF_SIZE;
    }
}

void call_back_send(uint8_t instance, uint8_t ch)
{
    while ((uart_list[instance]->STS &0x80) == 0)
        continue;
    uart_list[instance]->DT = ch;
}
