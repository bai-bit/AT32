/*
 * File      : drv_hwtimer.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2015, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author           Notes
 * 2015-09-02     heyuanjie87      the first version
 */

#include <rtthread.h>
#include <rtdevice.h>
#include "common.h"
#include "ctimer.h"

#ifdef RT_USING_HWTIMER

//static void NVIC_Configuration(void)
//{
//    NVIC_EnableIRQ(TIMER0_IRQn);
//}

static void timer_init(rt_hwtimer_t *timer, rt_uint32_t state)
{
    if (state == 1)
    {
        CTIMER_TC_Init(HW_CTIMER0, 1000*1000);
        CTIMER_Stop(HW_CTIMER0);
        CTIMER_TC_SetIntMode(HW_CTIMER0, HW_CTIMER_CH0, true);
    }
}

static rt_err_t timer_start(rt_hwtimer_t *timer, rt_uint32_t t, rt_hwtimer_mode_t opmode)
{
    CTIMER_TC_Init(HW_CTIMER0, t);
    return RT_EOK;
}

static void timer_stop(rt_hwtimer_t *timer)
{
//    LPC_TIM_TypeDef *tim;

//    tim = (LPC_TIM_TypeDef *)timer->parent.user_data;

//    TIM_Cmd(tim, DISABLE);
}

static rt_err_t timer_ctrl(rt_hwtimer_t *timer, rt_uint32_t cmd, void *arg)
{
//    LPC_TIM_TypeDef *tim;
    rt_err_t err = RT_EOK;

//    tim = (LPC_TIM_TypeDef *)timer->parent.user_data;

//    switch (cmd)
//    {
//    case HWTIMER_CTRL_FREQ_SET:
//    {
//        uint32_t clk;
//        uint32_t pre;

//        clk = CLKPWR_GetCLK(CLKPWR_CLKTYPE_PER);
//        pre = clk / *((uint32_t*)arg) - 1;
//        tim->PR = pre;
//    }
//    break;
//    default:
//    {
//        err = -RT_ENOSYS;
//    }
//    break;
//    }

    return err;
}

static rt_uint32_t timer_counter_get(rt_hwtimer_t *timer)
{
    return CTIMER_GetCounter(0);
}

static const struct rt_hwtimer_info _info =
{
    1000000,           /* the maximum count frequency can be set */
    2000,              /* the minimum count frequency can be set */
    0xFFFFFF,          /* the maximum counter value */
    HWTIMER_CNTMODE_UP,/* Increment or Decreasing count mode */
};

static const struct rt_hwtimer_ops _ops =
{
    timer_init,
    timer_start,
    timer_stop,
    timer_counter_get,
    timer_ctrl,
};

static rt_hwtimer_t _timer0;

int rt_hw_hwtimer_init(void)
{
    _timer0.info = &_info;
    _timer0.ops  = &_ops;

    rt_device_hwtimer_register(&_timer0, "timer0", NULL);

    return 0;
}

void CTIMER0_IRQHandler(void)
{
    CTIMER0->IR = CTIMER_IR_MR0INT_MASK;
    rt_device_hwtimer_isr(&_timer0);
}

//INIT_BOARD_EXPORT(lpc_hwtimer_init);
#endif
