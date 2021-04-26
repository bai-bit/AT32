#include <rtthread.h>
#include <rtdevice.h>
#include <stdio.h>
#include "common.h"
#include "timer.h"

#define NRF528XX_TIMER_CNT        (3)

struct nrf_hwtimer
{
    uint32_t instance;
};

struct nrf_hwtimer hwtimer[NRF528XX_TIMER_CNT];
static rt_hwtimer_t _timer[NRF528XX_TIMER_CNT];

static void timer_init(rt_hwtimer_t *timer, rt_uint32_t state)
{
    struct nrf_hwtimer *tim = (struct nrf_hwtimer *)timer->parent.user_data;
    
    if(state == 1)
    {
        TIMER_Init(tim->instance, HW_TIMER_CH0, 1000*1000);
        TIMER_Stop(tim->instance);
    }
}

static rt_err_t timer_start(rt_hwtimer_t *timer, rt_uint32_t t, rt_hwtimer_mode_t opmode)
{
    struct nrf_hwtimer *tim = (struct nrf_hwtimer *)timer->parent.user_data;
    
    TIMER_Start(tim->instance);
    return RT_EOK;
}

static void timer_stop(rt_hwtimer_t *timer)
{
    struct nrf_hwtimer *tim = (struct nrf_hwtimer *)timer->parent.user_data;
    
    TIMER_Stop(tim->instance);
}

static rt_err_t timer_ctrl(rt_hwtimer_t *timer, rt_uint32_t cmd, void *arg)
{
    struct nrf_hwtimer *tim = (struct nrf_hwtimer *)timer->parent.user_data;
    
    rt_err_t err = RT_EOK;

    switch (cmd)
    {
        case HWTIMER_CTRL_FREQ_SET:
        {
            rt_uint32_t freq;

            freq = *((rt_uint32_t*)arg);
            TIMER_SetTime(tim->instance, HW_TIMER_CH0, (1000*1000) / freq);
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
    struct nrf_hwtimer *tim = (struct nrf_hwtimer *)timer->parent.user_data;
    return TIMER_GetTime(tim->instance, HW_TIMER_CH0);
}


static const struct rt_hwtimer_info _info =
{
    1000000,           /* the maximum count frequency can be set */
    1,              /* the minimum count frequency can be set */
    0x0FFFFFFF,            /* the maximum counter value */
    HWTIMER_CNTMODE_DW,/* Increment or Decreasing count mode */
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

void TIMER0_IRQHandler(void)
{
    NRF_TIMER0->EVENTS_COMPARE[0] = 0;
    rt_device_hwtimer_isr(&_timer[0]);
}

void TIMER1_IRQHandler(void)
{
    NRF_TIMER1->EVENTS_COMPARE[0] = 0;
    rt_device_hwtimer_isr(&_timer[1]);
}

void TIMER2_IRQHandler(void)
{
    NRF_TIMER2->EVENTS_COMPARE[0] = 0;
    rt_device_hwtimer_isr(&_timer[2]);
}


