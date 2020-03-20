#include "common.h"
#include "led.h"
#include "uart.h"
#include "systemclk.h"
#include "exti.h"
#include "standby.h"
#include "tsic506.h"
#include<stdio.h>

void tsic_power(uint8_t status);
uint8_t tsic_data_output(void);

tsic_oper_t tsic_opt = {
    .tsic_delayus = delayus,
    .tsic_delayms = delayms,
    .tsic_power = tsic_power,
    .tsic_data_output = tsic_data_output,
};

void tsic_power(uint8_t status)
{
    PAout(8) = status;
}

uint8_t tsic_data_output(void)
{
    return PAin(7);
}
int main(int argc,const char *argv[])
{
    SysClk_HSIEN();
    RCC->CTRL &= (!HSEEN_BIT);
	DelayInit();

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    Tsic_Init(&tsic_opt);
    RCC->APB2EN |= AFIO_ENABLEBIT;
    AFIO->MAP |= 0x4;
    
    GPIO_Init(HW_GPIOB, GPIO_PIN_6, GPIO_Mode_AF_PP);
    GPIO_Init(HW_GPIOB, GPIO_PIN_7, GPIO_Mode_IN_FLOATING);
 	UART_Init(HW_USART1, BAUD_115200);

    GPIO_Init(HW_GPIOA, GPIO_PIN_2, GPIO_Mode_AF_PP);
    GPIO_Init(HW_GPIOA, GPIO_PIN_3, GPIO_Mode_IN_FLOATING);
    UART_Init(HW_USART2, BAUD_115200);

    GPIO_Init(HW_GPIOA,GPIO_PIN_9,GPIO_Mode_Out_PP);
    PAout(9) = 1;
    delayms(800);
    GPIO_Init(HW_GPIOA,GPIO_PIN_8,GPIO_Mode_Out_PP);
    PAout(8) = 0;
    GPIO_Init(HW_GPIOA,GPIO_PIN_7,GPIO_Mode_IN_FLOATING);

    GPIO_Init(HW_GPIOA, GPIO_PIN_0, GPIO_Mode_IPD);
    AFIO_Init(HW_GPIOA,GPIO_PIN_0);
    Exti_Init(exti_line0, interrupt, rising, ENABLE);
    NVIC_Init(EXTI0_IRQn, 3, 2, ENABLE);
    
    GPIO_Init(HW_GPIOA, GPIO_PIN_5, GPIO_Mode_Out_PP);
    PAout(5) = 1;

//    WKUP_Init();//ÅäÖÃ´ý»úÄ£Ê½
    uint8_t data[2] = "";
    double count = 0;
    uint16_t value = 0;

	while(1)
	{
//        delayms(200);
        
//        GPIO_PinWrite(HW_GPIOA,GPIO_PIN_5,0);
//        
        if(!(GPIOA->OPTDT & (0x1 << 5)))
        {
            delayms(5);
            if(read_tsic506_byte(&count))
                printf("read tsic data error\r\n");
            
            value = count * 1000;
            printf("%f\r\n",count);

            data[0] = ((uint8_t)value) & 0xff;
            data[1] = (uint8_t)((value & 0xff00) >> 8);
            
            UART_PutChar(HW_USART2, data[0]);
            UART_PutChar(HW_USART2, data[1]);
            GPIO_PinToggle(HW_GPIOA,GPIO_PIN_5);
        }
        

//        PWR_EnterSTANDBYMode();
	}
}

void EXTI0_IRQHandler(void)
{
//    double count = 0;
//    uint16_t value = 0;
//    uint8_t data[2] = "";
    Clean_ExtiInter(exti_line0);
//    
//    if(read_tsic506_byte(&count))
//                printf("read tsic data error\r\n");
//            
//            value = count * 1000;

//            data[0] = ((uint8_t)value) & 0xff;
//            data[1] = (uint8_t)((value & 0xff00) >> 8);
//            
//            UART_PutChar(HW_USART2, data[0]);
//            UART_PutChar(HW_USART2, data[1]);
    GPIO_PinWrite(HW_GPIOA,GPIO_PIN_5,0);
    GPIO_PinToggle(HW_GPIOA,GPIO_PIN_9);
}

void uart1_to_uart2(void)
{
    if (USART1->STS & USART_STS_RDNE)
    {
        USART2->DT = (USART1->DT & 0xFF);
        while (!((USART2->STS) & USART_STS_TRAC)); 
    }
    if (USART2->STS & USART_STS_RDNE)
    {
        USART1->DT = (USART2->DT & 0xFF);
        while (!((USART1->STS) & USART_STS_TRAC)); 
    }
}
