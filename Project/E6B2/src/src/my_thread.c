#include <rtthread.h>
#include <stdio.h>
#include <common.h>
#include <gpio_init.h>
#include <uart.h>
#include "imu_data_decode.h"
#include "packet.h"

#define rt_kprintf printf

static int timer_us;
static int timer_ms;
static float refer_value;
static rt_mutex_t dynamic_mutex = RT_NULL;

void rt_hw_us_delay(rt_uint32_t us)
{
    rt_uint32_t start, now, delta, reload, us_tick;
    start = SysTick->VAL;
    reload = SysTick->LOAD;
    us_tick = (SysTick->LOAD / (1000000 / RT_TICK_PER_SECOND));
    do{
        now = SysTick->VAL;
        delta = start > now ? start - now : reload + start - now;
    }while(delta < us_tick * us);
}

ALIGN(RT_ALIGN_SIZE);
void thread2_entry(void *parameter)
{
    uint32_t count = 0;
    uint32_t timeout = 0;
    uint32_t temp_timer_us = 0;
    uint32_t temp_timer_ms = 0;
    float temp_refer_value = 0;
    // 产生波形

    while(1)
    {
        rt_mutex_take(dynamic_mutex, 2);
        temp_timer_us = timer_us;
        temp_timer_ms = timer_ms;
        temp_refer_value = refer_value;

        rt_mutex_release(dynamic_mutex);
//                printf("refer_value = [%f]\r\n", refer_value);
//        printf("temp_refer_value = [%f]\r\n", temp_refer_value);
        //执行四次为一个周期
        if(temp_refer_value)
        {
            count++;

         
            if(count % 4 == 0)
            {
                if(temp_refer_value > 0)
                {
                    PBout(8) = 0;
                    PBout(9) = 1;
                }
                else if(temp_refer_value < 0)
                {
                    PBout(8) = 1;
                    PBout(9) = 0;
                }
            }
            else if(count % 4 == 1)
            {
                
                PBout(8) = 1;
                PBout(9) = 1;
            }
            else if(count % 4 == 2)
            {
                if(temp_refer_value > 0)
                {
                    PBout(8) = 1;
                    PBout(9) = 0;
                }
                else if(temp_refer_value < 0)
                {
                    PBout(8) = 0;
                    PBout(9) = 1;
                }
            }
            else if(count % 4 == 3)
            {
                PBout(8) = 0;
                PBout(9) = 0;
            }
            
            rt_thread_mdelay(temp_timer_ms / 1000);
            rt_hw_us_delay(temp_timer_ms % 1000);
        }
    }
}

void thread1_entry(void *parameter)
{
    //读取信息
    
    while(1)
    {
        refer_value = 0;
//        rt_mutex_take(dynamic_mutex, 2);
        if(receive_imusol.gyr[2] > 1 && receive_imusol.gyr[2] < 280)
        {
            refer_value = receive_imusol.gyr[2];
            timer_ms = 45000/refer_value ;
//            TMR2->CTRL1 = ~TMR_CTRL1_CNTEN;
            TMR2->AR = timer_ms;
            TMR2->CTRL1 = TMR_CTRL1_CNTEN;
//            printf("timer_ms = [%d]\r\n", timer_ms);
        }
        else if(receive_imusol.gyr[2] < -1 && receive_imusol.gyr[2] > -280)
        {
            refer_value = (receive_imusol.gyr[2]);
            timer_ms = 45000/refer_value * -1;
            TMR2->AR = timer_ms;
            TMR2->CTRL1 = TMR_CTRL1_CNTEN;
//            printf("timer_ms = [%d]\r\n", timer_ms);
        }
        else
        {
            TMR2->CTRL1 = ~TMR_CTRL1_CNTEN;
        }
//        rt_mutex_release(dynamic_mutex);
        rt_thread_mdelay(10);
    }

}

void thread3_entry(void* parameter)
{
    printf("3 ok");
    int i = 0;
    char *p = NULL;
    for (i = 0; ; i++)
    {
        p = rt_malloc(1 << i);
        if(p != NULL)
        {
            printf("get memory [%d]\r\n", 1 << i);
            
            rt_free(p);
            
            printf("free memory [%d]\r\n", 1 << i);
            
            p = NULL;
            rt_thread_delay(50);
        }
        else 
        {
            printf("malloc is error : %d\r\n", 1 << i);
           while(1);
        }
    }
}
    
#define THREAD_STACK_SIZE 1024
#define THREAD_PRIORITY   21
#define THREAD_TIMESLICE  20
void init_thread_entry(void* parameter)
{
    static rt_thread_t tid1 = RT_NULL,tid2 = RT_NULL;
    UART_Init(HW_USART1, 115200);
    
//    printf("ok");
//    dynamic_mutex = rt_mutex_create("dmutex", RT_IPC_FLAG_FIFO);
//    if(dynamic_mutex == RT_NULL)
//    {
//        rt_kprintf("create dynamic mutex faild\r\n");
//        return;
//    }
    
    tid1 = rt_thread_create("t1", thread1_entry, RT_NULL, 1024,21,20);
    if(tid1 != RT_NULL)
        rt_thread_startup(tid1);
//    tid2 = rt_thread_create("t2", thread2_entry, RT_NULL, THREAD_STACK_SIZE, THREAD_PRIORITY,THREAD_TIMESLICE);
//    if(tid2 != RT_NULL)
//        rt_thread_startup(tid2);
//    rt_thread_startup(rt_thread_create("t3", thread3_entry, RT_NULL, 1024, 21, 20));
//    rt_thread_delete(rt_thread_self());
    
//    rt_timer_start(rt_timer_create("timer1", thread1_entry, RT_NULL, 10, RT_TIMER_FLAG_ACTIVATED));
//    tmr = rt_timer_create("tmr", tmr_cb, RT_NULL, 2000, RT_TIMER_FLAG_PERIODIC);
//    rt_timer_start(tmr);
}
//static float temp_refer_value = 0;
//void TMR2_GLOBAL_IRQHandler(void)
//{
//    
//    static uint32_t count = 0;
////    uint32_t timeout = 0;

//////    rt_mutex_take(dynamic_mutex, 2);
//    temp_refer_value = refer_value;
//////    rt_mutex_release(dynamic_mutex);
////    
//    TMR2->STS = 0;
////    GPIO_PinToggle(HW_GPIOA,GPIO_PIN_9);
////    
//    if(temp_refer_value)
//    {
//        count++;
//        if(count % 16 == 0 || count % 16 == 1 || count % 16 == 2 || count % 16 == 3)
//        {
//            if(temp_refer_value > 0)
//            {
//                PBout(8) = 0;
//                PBout(9) = 1;
//            }
//            else if(temp_refer_value < 0)
//            {
//                PBout(8) = 1;
//                PBout(9) = 0;
//            }
//        }
//        else if(count % 16 == 4 || count % 16 == 5 || count % 16 == 6 || count % 16 == 7)
//        {
//            PBout(8) = 1;
//            PBout(9) = 1;
//        }
//        else if(count % 16 == 8 || count % 16 == 9 || count % 16 == 10 || count % 16 == 11)
//        {
//             if(temp_refer_value > 0)
//            {
//                PBout(8) = 1;
//                PBout(9) = 0;
//            }
//            else if(temp_refer_value < 0)
//            {
//                PBout(8) = 0;
//                PBout(9) = 1;
//            }
//        }
//        else if(count % 16 == 12 || count % 16 == 13 || count % 16 == 14 || count % 16 == 15)
//        {
//            PBout(8) = 0;
//            PBout(9) = 0;
//        }
//    }
//}

