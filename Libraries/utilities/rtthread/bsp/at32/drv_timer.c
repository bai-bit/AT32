#include <rtthread.h>
#include <rtdevice.h>
#include <stdio.h>
#include "common.h"
#include "at32_timer.h"

#define AT32_TIMER_CNT        (3)

struct at32_hwtimer
{
    uint32_t instance;
};

struct at32_hwtimer hwtimer[AT32_TIMER_CNT];
static rt_hwtimer_t _timer[AT32_TIMER_CNT];

static void timer_init(rt_hwtimer_t *timer, rt_uint32_t state)
{
    struct at32_hwtimer *tim = (struct at32_hwtimer *)timer->parent.user_data;
    
    if(state == 1)
    {
        TIMER_Init(tim->instance, HW_TIMER_CH0, 1000*1000);
        TIMER_Stop(tim->instance);
    }
}

static rt_err_t timer_start(rt_hwtimer_t *timer, rt_uint32_t t, rt_hwtimer_mode_t opmode)
{   
    struct at32_hwtimer *tim = (struct at32_hwtimer *)timer->parent.user_data;
    TIMER_Start(tim->instance);
    return RT_EOK;
}

static void timer_stop(rt_hwtimer_t *timer)
{
    struct at32_hwtimer *tim = (struct at32_hwtimer *)timer->parent.user_data;
    TIMER_Stop(tim->instance);
}

static rt_err_t timer_ctrl(rt_hwtimer_t *timer, rt_uint32_t cmd, void *arg)
{
    struct at32_hwtimer *tim = (struct at32_hwtimer *)timer->parent.user_data;
    
    rt_err_t err = RT_EOK;

    switch (cmd)
    {
        case HWTIMER_CTRL_FREQ_SET:
        {
//            rt_uint32_t frq;
//            frq = *((rt_uint32_t*)arg);
            //rt_kprintf("%s instance:%d, cmd:%d frq:%d\r\n", __FUNCTION__, tim->instance, cmd, frq);
            
           // TIMER_SetTime(tim->instance, HW_TIMER_CH0, (1000*1000) / frq);
            TIMER_SetIntMode(tim->instance, HW_TIMER_CH0, true);
            TIMER_Start(tim->instance);
        }
        break;
        case HWTIMER_CTRL_STOP:
            timer_stop(timer);
            break;
        default:
        {
            err = -RT_ENOSYS;
        }
        break;
    }

    return err;
}

static rt_uint32_t timer_counter_get(rt_hwtimer_t *timer)
{
//    struct at32_hwtimer *tim = (struct at32_hwtimer *)timer->parent.user_data;
   // return TIMER_GetTime(tim->instance, HW_TIMER_CH0);
    return 0;
}

static const struct rt_hwtimer_info _info =
{
    1000000,                /* the maximum count frequency can be set */
    1,                      /* the minimum count frequency can be set */
    0x0FFFFFFF,             /* the maximum counter value */
    HWTIMER_CNTMODE_DW,     /* Increment or Decreasing count mode */
};

static const struct rt_hwtimer_ops _ops =
{
    timer_init,
    timer_start,
    timer_stop,
    timer_counter_get,
    timer_ctrl,
};

int rt_hw_hwtimer_init(const char *name)
{
    uint32_t instance;
    
    sscanf(name, "timer%d", &instance);
    
    _timer[instance].info = &_info;
    _timer[instance].ops  = &_ops;
    
    hwtimer[instance].instance = instance;
    
    rt_device_hwtimer_register(&_timer[instance], name, &hwtimer[instance]);
    return 0;
}


void TMR2_GLOBAL_IRQHandler(void)
{
    TMR2->STS &= ~TMR_STS_UEVIF;
    rt_device_hwtimer_isr(&_timer[2]);
}

//void TMR3_GLOBAL_IRQHandler(void)
//{
//    TMR3->STS &= ~TMR_STS_UEVIF;
//    rt_device_hwtimer_isr(&_timer[3]);
//}

//void TMR4_GLOBAL_IRQHandler(void)
//{
//    TMR4->STS &= ~TMR_STS_UEVIF;
//    rt_device_hwtimer_isr(&_timer[4]);
//}


//void TMR5_GLOBAL_IRQHandler(void)
//{
//    TMR5->STS &= ~TMR_STS_UEVIF;
//    rt_device_hwtimer_isr(&_timer[5]);
//}

