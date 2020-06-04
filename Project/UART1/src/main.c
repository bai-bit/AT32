#include "common.h"
#include "led.h"
#include "uart.h"
#include "systemclk.h"
#include "exti.h"
#include <stdio.h>

uint16_t crc_update(const uint8_t *src, uint32_t LengthInBytes);

int main(int argc,const char *argv[])
{
    SysClk_HSIEN();

	DelayInit();

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    
    /* serial */
    RCC->APB2EN |= AFIO_ENABLEBIT;
    AFIO->MAP |= AFIO_MAP_USART1_REMAP ;
    GPIO_Init(HW_GPIOB, GPIO_PIN_6, GPIO_Mode_AF_PP);
    GPIO_Init(HW_GPIOB, GPIO_PIN_7, GPIO_Mode_IN_FLOATING);
    UART_Init(HW_USART1, BAUD_115200);
    
    /* 1A0  PB5 */
    GPIO_Init(HW_GPIOB, GPIO_PIN_5, GPIO_Mode_Out_PP);
    PBout(5) = 1;
    /* 1A1  PB4 */
    GPIO_Init(HW_GPIOB, GPIO_PIN_4, GPIO_Mode_Out_PP);
    PBout(4) = 1;
    /* 1A2  PB3 */
    GPIO_Init(HW_GPIOB, GPIO_PIN_3, GPIO_Mode_Out_PP);
    PBout(3) = 1;


    /* 2A0  PA15 */
    GPIO_Init(HW_GPIOA, GPIO_PIN_15, GPIO_Mode_Out_PP);
    PAout(15) = 1;
    /* 2A1  PA12 */
    GPIO_Init(HW_GPIOA, GPIO_PIN_12, GPIO_Mode_Out_PP);
    PAout(12) = 1;
    /* 2A2  PA11 */
    GPIO_Init(HW_GPIOA, GPIO_PIN_11, GPIO_Mode_Out_PP);
    PAout(11) = 1;
    
    /* 3A0  PA10 */
    GPIO_Init(HW_GPIOA, GPIO_PIN_10, GPIO_Mode_Out_PP);
    PAout(10) = 1;
    /* 3A1  PA9 */
    GPIO_Init(HW_GPIOA, GPIO_PIN_9, GPIO_Mode_Out_PP);
    PAout(9) = 1;
    /* 3A2  PA8 */
    GPIO_Init(HW_GPIOA, GPIO_PIN_8, GPIO_Mode_Out_PP);
    PAout(8) = 1;
    

    /* 4A0  PB15 */
    GPIO_Init(HW_GPIOB, GPIO_PIN_15, GPIO_Mode_Out_PP);
    PBout(15) = 1;
    /* 4A1  PB14 */
    GPIO_Init(HW_GPIOB, GPIO_PIN_14, GPIO_Mode_Out_PP);
    PBout(14) = 1;
    /* 4A2 PB13 */
    GPIO_Init(HW_GPIOB, GPIO_PIN_13, GPIO_Mode_Out_PP);
    PBout(13) = 1;
   
    
    /* EN4  PB8*/
    GPIO_Init(HW_GPIOB, GPIO_PIN_8, GPIO_Mode_Out_PP);
    PBout(8) = 1;
    /* EN3  PB9 */
    GPIO_Init(HW_GPIOB, GPIO_PIN_9, GPIO_Mode_Out_PP);
    PBout(9) = 1;
    /* EN7  PB10 */
    GPIO_Init(HW_GPIOB, GPIO_PIN_10, GPIO_Mode_Out_PP);
    PBout(10) = 1;
    /* EN6  PB11 */
    GPIO_Init(HW_GPIOB, GPIO_PIN_11, GPIO_Mode_Out_PP);
    PBout(11) = 1;
    /* EN5  PB12 */
    GPIO_Init(HW_GPIOB, GPIO_PIN_12, GPIO_Mode_Out_PP);
    PBout(12) = 1;
    /* EN2  PA3 */
    GPIO_Init(HW_GPIOA, GPIO_PIN_3, GPIO_Mode_Out_PP);
    PAout(3) = 1;
    PBout(11) = 1;
    /* EN1  PA4 */
    GPIO_Init(HW_GPIOA, GPIO_PIN_4, GPIO_Mode_Out_PP);
    PAout(4) = 1;
    /* EN8  PB1 */
    GPIO_Init(HW_GPIOB, GPIO_PIN_1, GPIO_Mode_Out_PP);
    PBout(1) = 1;
 
    AFIO->MAP |= AFIO_MAP_SWJTAG_CONF_ALLDISABLE;
 

    int i = 0;
	while(1)
	{
//        /* 4 */
//        PBout(13) = 0;
//        PBout(14) = 0;
//        PBout(15) = 0;
//        
//        /* 3 */
//        PAout(8) = 0;
//        PAout(9) = 0;
//        PAout(10) = 0;
//        
//        /* 2 */
//        PAout(11) = 0;
//        PAout(12) = 0;
//        PAout(15) = 0;
//        
//        /* 1 */
//        PBout(3) = 0;
//        PBout(4) = 0;
//        PBout(5) = 0;

//        PBout(12) = 0;
//        PAout(4) = 0;
//        delayms(20);
// 
//            /* 1  connect failed*/
//            PBout(5) =  1;
//        
//            PBout(4) = 1;
//            
//            PBout(3) = 1;
//            
//            delayms(8000);
//            
//            /* 2  connect failed*/
//            PBout(5) =  0;
//        
//            PBout(4) = 1;
//            
//            PBout(3) = 1;
//            
//            delayms(8000);
//            
//            /* 3 connect success*/
//            PBout(5) =  1;
//        
//            PBout(4) = 0;
//            
//            PBout(3) = 1;
//            
//            delayms(8000);
//            
//            /* 4 connect success*/
//            PBout(5) =  0;
//        
//            PBout(4) = 0;
//            
//            PBout(3) = 1;
//            
//            delayms(8000);
//            
//            /* 5  connect success*/
//            PBout(5) =  1;
//        
//            PBout(4) = 1;
//            
//            PBout(3) = 0;
//            
//            delayms(8000);
//            
//            /* 6  connect failed*/
//            PBout(5) =  0;
//        
//            PBout(4) = 1;
//            
//            PBout(3) = 0;
//            
//            delayms(8000);
//            
//            /* 7  connect success*/
//            PBout(5) =  1;
//        
//            PBout(4) = 0;
//            
//            PBout(3) = 0;
//            
//            delayms(8000);
//            
//            /* 8  connect success*/
//            PBout(5) =  0;
//        
//            PBout(4) = 0;
//            
//            PBout(3) = 0;
//            
//            delayms(8000);

//        PBout(12) = 1;
//        PAout(4) = 1;
//        
//        PAout(3) = 0;
//        PBout(11) = 0;
//        delayms(20);
//        /* 1  ok*/
//        PAout(15) = 1;
//        PAout(12) = 1;
//        PAout(11) = 1;
//        delayms(8000);
//        
//        /* 2  ok*/
//        PAout(15) = 0;
//        PAout(12) = 1;
//        PAout(11) = 1;
//        delayms(8000);
//        
//        /* 3  ok*/
//        PAout(15) = 1;
//        PAout(12) = 0;
//        PAout(11) = 1;
//        delayms(8000);
//        
//        /* 4  not*/
//        PAout(15) = 0;
//        PAout(12) = 0;
//        PAout(11) = 1;
//        delayms(8000);
//        
//        /* 5  ok*/
//        PAout(15) = 1;
//        PAout(12) = 1;
//        PAout(11) = 0;
//        delayms(8000);
//        
//        /* 6  ok*/
//        PAout(15) = 1;
//        PAout(12) = 0;
//        PAout(11) = 0;
//        delayms(8000);
//        
//        /* 7  ok*/
//        PAout(15) = 0;
//        PAout(12) = 1;
//        PAout(11) = 0;
//        delayms(8000);
//        
//        /* 8  ok*/
//        PAout(15) = 0;
//        PAout(12) = 0;
//        PAout(11) = 0;
//        delayms(8000);
//        
//        PAout(3) = 1;
//        PBout(11) = 1;

printf("11");

	}
}





