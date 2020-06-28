#include <stdio.h>
#include "common.h"
#include "gpio.h"
#include "uart.h"
#include "rtthread.h"
#include "rthw.h"


#if (APP_PN == 229)
#define SYSTEM_HEAP_BASE        (0x20002000)
#else
#define SYSTEM_HEAP_BASE        (0x20008000)
#endif

#define SYSTEM_HEAP_SIZE        (24*1024)


void init_thread_entry(void* parameter);

void rt_application_init(void* parameter)
{
    rt_thread_startup(rt_thread_create("init", init_thread_entry, RT_NULL, 1024, 21, 20));
}

int main(void)
{
    int ret;
    
    ret = SetupSystemPLL(1, 96*1000*1000);
    
    /* disable SWD */
    RCC->APB2EN |= RCC_APB2EN_AFIOEN;
    #if defined(AT32F413Cx_HD )
    //AFIO->MAP7 |= (4<<16);
    AFIO->MAP7 |= (2<<16);
    #elif defined(AT32F403Zx_HD )
    AFIO->MAP |= (2<<24);
    #endif
    
    DelayInit();
    
    SetPinMux(HW_GPIOB, 6, 2, 3);
    SetPinMux(HW_GPIOB, 7, 2, 0);

//    GPIO_SetPinPull(HW_GPIOB, 6, 1);
    RCC->APB2EN |= 1;
    AFIO->MAP |= AFIO_MAP_USART1_REMAP; /* REMAP USART1 to PB6 PB7 */
    UART_Init(HW_UART1,115200);
    
    rt_hw_interrupt_disable();

    SysTick_Config(GetClock(kAHBClock) / RT_TICK_PER_SECOND - 1);
    
    rt_system_timer_init();
    rt_system_scheduler_init();

    rt_device_init_all();
    rt_system_timer_thread_init();
    rt_thread_idle_init();
    rt_system_heap_init((void*)SYSTEM_HEAP_BASE, (void*)(SYSTEM_HEAP_BASE + SYSTEM_HEAP_SIZE));  
    rt_application_init(RT_NULL);
    rt_system_scheduler_start();
}

void DelayMs(uint32_t ms)
{
    rt_thread_delay(rt_tick_from_millisecond(ms));
}

void rt_hw_us_delay(rt_uint32_t us)
{
    rt_uint32_t delta;
    us = us * (SysTick->LOAD / (1000000 / RT_TICK_PER_SECOND));
    delta = SysTick->VAL;
    while (delta - SysTick->VAL < us);
}

void DelayUs(uint32_t us)
{
    rt_hw_us_delay(us);
} 

void SysTick_Handler(void)
{
    rt_interrupt_enter();
    rt_tick_increase();
    rt_interrupt_leave();
}
