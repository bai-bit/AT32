#include "rtthread.h"
#include <stdio.h>
#include "common.h"
#include "gpio_init.h"
#include "uart.h"
#include "tsic506.h"
#include "standby.h"
#include "drv_gpio.h"

/* defined the LED2 pin: PD13 */
#define LED2_PIN    GET_PIN(D, 13)
/* defined the LED3 pin: PD14 */
#define LED3_PIN    GET_PIN(D, 14)
/* defined the LED4 pin: PD15 */
#define LED4_PIN    GET_PIN(D, 15)

#define ONE_THOUSAND_TIMES (1000)
#define TSIC506_BREAKDOWN 65.535
#define THREAD_STACK_SIZE 1024
#define THREAD_PRIORITY   21
#define THREAD_TIMESLICE  20

void tsic_power(uint8_t status);

uint8_t tsic_data_output(void);
void rt_hw_us_delay(uint32_t us);

tsic_oper_t tsic_opt = {
    .tsic_delayus = rt_hw_us_delay,
    .tsic_delayms = rt_thread_mdelay,
    .tsic_power = tsic_power,
    .tsic_data_output = tsic_data_output,
};

void rt_hw_us_delay(uint32_t us)
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

void tsic_power(uint8_t status)
{
    GPIO_PinWrite(HW_GPIOB, GPIO_PIN_5, status);
}

uint8_t tsic_data_output(void)
{
    return PBin(8);
}

void thread1_entry(void *parameter)
{
    uint8_t data[2] = "";
    double count = 0;
    uint16_t value = 0;
    int ret = 0;

//    WKUP_Init();
    /*initialize tsic506*/
    Tsic_Init(&tsic_opt);
    
    rt_device_open(rt_device_find("uart2"), RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_STREAM);
    printf("ok");
   
    while(1)
    {
        GPIO_PinWrite(HW_GPIOA, GPIO_PIN_5, 0);

        ret = 0;
        if(!(GPIOA->OPTDT & (0x1 << 5)))
        {
            rt_thread_mdelay(6);
            
            ret = read_tsic506_byte(&count);
            if(ret)
            {
                LIB_TRACE("ret = [%d] \r\n", ret);
                LIB_TRACE("read tsic data error\r\n");
                PBout(5) = 0;
            }
            
            if(count < -10 || count > 60)
            {
                ret = read_tsic506_byte(&count);
                if(ret)
                {
                    LIB_TRACE("ret = [%d] \r\n", ret);
                    LIB_TRACE("read tsic data error\r\n");
                    PBout(5) = 0;
                }
            }
            
            if(count < -10 || count > 60)
                count = TSIC506_BREAKDOWN;
            
            value = count * ONE_THOUSAND_TIMES;

            LIB_TRACE("%f\r\n",count);

            data[0] = ((uint8_t)value) & 0xff;
            data[1] = (uint8_t)((value & 0xff00) >> 8);
            
            UART_PutChar(HW_USART2, data[0]);
            UART_PutChar(HW_USART2, data[1]);
            GPIO_PinToggle(HW_GPIOA, GPIO_PIN_5);
            
        }

//        gpio_AINMode();
//        PWR_EnterSTANDBYMode();
    }
}

void thread2_entry(void *parameter)
{
    uint32_t Speed = 200;
    /* set LED2 pin mode to output */
    rt_pin_mode(LED2_PIN, PIN_MODE_OUTPUT);
    /* set LED3 pin mode to output */
    rt_pin_mode(LED3_PIN, PIN_MODE_OUTPUT);
    /* set LED4 pin mode to output */
    rt_pin_mode(LED4_PIN, PIN_MODE_OUTPUT);
    
    while(1)
    {
        rt_pin_write(LED2_PIN, PIN_LOW);
        rt_thread_mdelay(Speed);
        rt_pin_write(LED3_PIN, PIN_LOW);
        rt_thread_mdelay(Speed);
        rt_pin_write(LED4_PIN, PIN_LOW);
        rt_thread_mdelay(Speed);
        rt_pin_write(LED2_PIN, PIN_HIGH);
        rt_thread_mdelay(Speed);
        rt_pin_write(LED3_PIN, PIN_HIGH);
        rt_thread_mdelay(Speed);
        rt_pin_write(LED4_PIN, PIN_HIGH);
        rt_thread_mdelay(Speed);
    }
        
}

void init_thread_entry(void* parameter)
{
    static rt_thread_t tid1 = RT_NULL,tid2 = RT_NULL;

    tid1 = rt_thread_create("t1", thread1_entry, RT_NULL, 1024, 10, THREAD_TIMESLICE);
    if(tid1 != RT_NULL)
        rt_thread_startup(tid1);

    tid2 = rt_thread_create("t2", thread2_entry, RT_NULL, 512, 9, THREAD_TIMESLICE);
    if(tid2 != RT_NULL)
        rt_thread_startup(tid2);
}

