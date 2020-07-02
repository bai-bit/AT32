#include "systemclk.h"
#include "common.h"
#include "gpio_init.h"
#include "uart.h"
#include "rtthread.h"
#include "rthw.h"
#include <drv_usart.h>

#define SAMPLE_UART_NAME "uart1"

#define SYSTEM_HEAP_BASE        (0x20002000)
#define SYSTEM_HEAP_SIZE        (24*1024)

void init_thread_entry(void* parameter);

void system_init(void);
void serial_gpio_init(void);
void tsic_port_init(void);

void rt_application_init(void* parameter)
{
    rt_components_init();
    rt_thread_startup(rt_thread_create("init", init_thread_entry, RT_NULL, 1024, 6, 20));
}

int main(void)
{
    SysClk_PLLEN(PLLCLK_MUL_192MHz);
  
    DelayInit();
    
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

    serial_gpio_init();
    tsic_port_init();
    GPIO_Init(HW_GPIOA, GPIO_PIN_9, GPIO_Mode_Out_PP);
    PAout(9) = 1;

//    printf("ok");
//    printf("%s %d %s\r\n", __FILE__, __LINE__, __FUNCTION__);
//    while(1)
//        continue;
    system_init();
}

void tsic_port_init(void)
{
    /* tsic506 power */
    GPIO_Init(HW_GPIOB, GPIO_PIN_5, GPIO_Mode_Out_PP);
    GPIO_resetSpeed(HW_GPIOB, GPIO_PIN_5, GPIO_Speed_2MHz);
    GPIO_PinWrite(HW_GPIOB, GPIO_PIN_5, 0);

    /* TSIC506 DATA */
    GPIO_Init(HW_GPIOB, GPIO_PIN_8, GPIO_Mode_IPU);
    
    /* signal gpio  */
    GPIO_Init(HW_GPIOA, GPIO_PIN_5, GPIO_Mode_Out_PP);
    PAout(5) = 1;
}

void system_init(void)
{
    rt_hw_interrupt_disable();

    SysTick_Config(GetClock_Frequency(pll) / RT_TICK_PER_SECOND - 1);

    rt_hw_usart_init(SAMPLE_UART_NAME);
    rt_console_set_device(SAMPLE_UART_NAME);
   
    rt_show_version();
    rt_system_timer_init();
    rt_system_scheduler_init();
    
    rt_device_init_all();
    rt_system_timer_thread_init();
    rt_thread_idle_init();
    rt_system_heap_init((void*)SYSTEM_HEAP_BASE, (void*)(SYSTEM_HEAP_BASE + SYSTEM_HEAP_SIZE));  
    rt_application_init(RT_NULL);
    
    rt_system_scheduler_start();
}

void serial_gpio_init(void)
{
    /* serial  usart1   afio  PB6 ---> TX, PB7 ---> RX  */
    RCC->APB2EN |= AFIO_ENABLEBIT;
    AFIO->MAP |= AFIO_MAP_USART1_REMAP;
    GPIO_Init(HW_GPIOB, GPIO_PIN_6, GPIO_Mode_AF_PP);
    GPIO_Init(HW_GPIOB, GPIO_PIN_7, GPIO_Mode_IPU);
//    UART_Init(HW_USART1, BAUD_115200);
//    USART_Cmd(HW_USART1, ENABLE);

    /* serial  usart2   PA2 ---> TX, PA3 ---> RX */
    GPIO_Init(HW_GPIOA, GPIO_PIN_2, GPIO_Mode_AF_PP);
    GPIO_Init(HW_GPIOA, GPIO_PIN_3, GPIO_Mode_IN_FLOATING);
}

void SysTick_Handler(void)
{
    rt_interrupt_enter();
    rt_tick_increase();
    rt_interrupt_leave();
} 
