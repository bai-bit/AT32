/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author            Notes
 * 2020-01-07     shelton           first version
 */

#include "drv_gpio.h"
#include "gpio_init.h"
#include "exti.h"
#include "common.h"

#ifdef RT_USING_PIN

static struct rt_pin_irq_hdr pin_irq_hdr_tab[] =
{
    {-1, 0, RT_NULL, RT_NULL},
    {-1, 0, RT_NULL, RT_NULL},
    {-1, 0, RT_NULL, RT_NULL},
    {-1, 0, RT_NULL, RT_NULL},
    {-1, 0, RT_NULL, RT_NULL},
    {-1, 0, RT_NULL, RT_NULL},
    {-1, 0, RT_NULL, RT_NULL},
    {-1, 0, RT_NULL, RT_NULL},
    {-1, 0, RT_NULL, RT_NULL},
    {-1, 0, RT_NULL, RT_NULL},
    {-1, 0, RT_NULL, RT_NULL},
    {-1, 0, RT_NULL, RT_NULL},
    {-1, 0, RT_NULL, RT_NULL},
    {-1, 0, RT_NULL, RT_NULL},
    {-1, 0, RT_NULL, RT_NULL},
    {-1, 0, RT_NULL, RT_NULL},
};
static uint32_t pin_irq_enable_mask=0;

static void at32_pin_write(rt_device_t dev, rt_base_t pin, rt_base_t value)
{
    uint8_t gpiox;
    
    gpiox = pin / 16;
    pin %= 16;
    
    if(value)
        value = 1;
    else
        value = 0;
    printf("11\r\n");
    GPIO_PinWrite(gpiox, pin, value);
}

static int at32_pin_read(rt_device_t dev, rt_base_t pin)
{
    uint8_t gpiox = 0;
    
    gpiox = pin / 16;
    pin = pin % 16;

    return read_gpioport(gpiox, pin);
}

static void at32_pin_mode(rt_device_t dev, rt_base_t pin, rt_base_t mode)
{
    const struct pin_index *index;
    uint8_t gpiox = 0;
    
    gpiox = pin / 16;
    pin = pin % 16; 

    switch(mode)
    {
        case PIN_MODE_INPUT :
            mode = GPIO_Mode_IN_FLOATING;
            break;
        case PIN_MODE_OUTPUT:
            mode = GPIO_Mode_Out_PP;
            break;
        case PIN_MODE_INPUT_PULLDOWN:
            mode = GPIO_Mode_IPD;
            break;
        case PIN_MODE_INPUT_PULLUP:
            mode = GPIO_Mode_IPU;
            break;
        case PIN_MODE_OUTPUT_OD:
            mode = GPIO_Mode_Out_OD;
            break;
        default:
            mode = GPIO_Mode_AIN;
    }
    
    GPIO_Init(gpiox, pin, mode);
}



static rt_err_t at32_pin_attach_irq(struct rt_device *device, rt_int32_t pin,
                                     rt_uint32_t mode, void (*hdr)(void *args), void *args)
{
    const struct pin_index *index;
    rt_base_t level;

    pin %= 16;

    level = rt_hw_interrupt_disable();
    if (pin_irq_hdr_tab[pin].pin == pin &&
            pin_irq_hdr_tab[pin].hdr == hdr &&
            pin_irq_hdr_tab[pin].mode == mode &&
            pin_irq_hdr_tab[pin].args == args)
    {
        rt_hw_interrupt_enable(level);
        return RT_EOK;
    }
    if (pin_irq_hdr_tab[pin].pin != -1)
    {
        rt_hw_interrupt_enable(level);
        return RT_EBUSY;
    }
    pin_irq_hdr_tab[pin].pin = pin;
    pin_irq_hdr_tab[pin].hdr = hdr;
    pin_irq_hdr_tab[pin].mode = mode;
    pin_irq_hdr_tab[pin].args = args;
    rt_hw_interrupt_enable(level);

    return RT_EOK;
}

static rt_err_t at32_pin_dettach_irq(struct rt_device *device, rt_int32_t pin)
{
    const struct pin_index *index;
    rt_base_t level;

    pin %= 16;

    level = rt_hw_interrupt_disable();
    if (pin_irq_hdr_tab[pin].pin == -1)
    {
        rt_hw_interrupt_enable(level);
        return RT_EOK;
    }
    pin_irq_hdr_tab[pin].pin = -1;
    pin_irq_hdr_tab[pin].hdr = RT_NULL;
    pin_irq_hdr_tab[pin].mode = 0;
    pin_irq_hdr_tab[pin].args = RT_NULL;
    rt_hw_interrupt_enable(level);

    return RT_EOK;
}

static rt_err_t at32_pin_irq_enable(struct rt_device *device, rt_base_t pin, rt_uint32_t enabled)
{
    const struct pin_index *index;
    uint8_t trigger_mode = 0;
    uint8_t exti_mode = 0;
    uint8_t gpiox = pin / 16;
    uint8_t mode = GPIO_Mode_IN_FLOATING;
    uint8_t NVIC_IRQChannel = 0;
    
    pin %= 16;
    
    rt_base_t level;


    level = rt_hw_interrupt_disable();//πÿ±’÷–∂œ

    if (pin_irq_hdr_tab[pin].pin == -1)
    {
        rt_hw_interrupt_enable(level);
        return RT_ENOSYS;
    }
    
    switch (pin_irq_hdr_tab[pin].mode)
    {
    case PIN_IRQ_MODE_RISING:
        trigger_mode = rising;
        break;
    case PIN_IRQ_MODE_FALLING:
        trigger_mode = failling;
        break;
    case PIN_IRQ_MODE_RISING_FALLING:
        trigger_mode = failling_and_rising;
        break;
    }

    switch(pin)
    {
        case 0:
            NVIC_IRQChannel = EXTI0_IRQn;
            break;
        case 1:
            NVIC_IRQChannel = EXTI1_IRQn;
            break;
        case 2:
            NVIC_IRQChannel = EXTI2_IRQn;
            break;
        case 3:
            NVIC_IRQChannel = EXTI3_IRQn;
            break;
        case 4:
            NVIC_IRQChannel = EXTI4_IRQn;
            break;
        case 5:
        case 6:
        case 7:
        case 8:
        case 9:
            NVIC_IRQChannel = EXTI9_5_IRQn;
            break;
        case 10:
        case 11:
        case 12:
        case 13:
        case 14:
        case 15:
            NVIC_IRQChannel = EXTI15_10_IRQn;
            break;
        default:
            break;
    }

    rt_hw_interrupt_enable(level);


    GPIO_Init(gpiox, pin, mode);
    AFIO_Init(gpiox, pin);
    Exti_Init(gpiox, exti_mode, trigger_mode, enabled);
    NVIC_Init(NVIC_IRQChannel, 3, 2, enabled);

    return RT_EOK;
}
const static struct rt_pin_ops _at32_pin_ops =
{
    at32_pin_mode,
    at32_pin_write,
    at32_pin_read,
    at32_pin_attach_irq,
    at32_pin_dettach_irq,
    at32_pin_irq_enable,
};

rt_inline void pin_irq_hdr(int irqno)
{
    Clean_ExtiInter(irqno);
    if (pin_irq_hdr_tab[irqno].hdr)
    {
        pin_irq_hdr_tab[irqno].hdr(pin_irq_hdr_tab[irqno].args);
    }
}

void GPIO_EXTI_IRQHandler(uint16_t GPIO_Pin)
{
    pin_irq_hdr(GPIO_Pin);
}

void EXTI0_IRQHandler(void)
{
    rt_interrupt_enter();
    GPIO_EXTI_IRQHandler(0);
    rt_interrupt_leave();
}

void EXTI1_IRQHandler(void)
{
    rt_interrupt_enter();
    GPIO_EXTI_IRQHandler(1);
    rt_interrupt_leave();
}

void EXTI2_IRQHandler(void)
{
    rt_interrupt_enter();
    GPIO_EXTI_IRQHandler(2);
    rt_interrupt_leave();
}

void EXTI3_IRQHandler(void)
{
    rt_interrupt_enter();
    GPIO_EXTI_IRQHandler(3);
    rt_interrupt_leave();
}

void EXTI4_IRQHandler(void)
{
    rt_interrupt_enter();
    GPIO_EXTI_IRQHandler(4);
    rt_interrupt_leave();
}

void EXTI9_5_IRQHandler(void)
{
    rt_interrupt_enter();
    if(RESET != Get_ExtiInter(5))
    {
        GPIO_EXTI_IRQHandler(5);
    }
    if(RESET != Get_ExtiInter(6))
    {
        GPIO_EXTI_IRQHandler(6);
    }
    if(RESET != Get_ExtiInter(7))
    {
        GPIO_EXTI_IRQHandler(7);
    }
    if(RESET != Get_ExtiInter(8))
    {
        GPIO_EXTI_IRQHandler(8);
    }
    if(RESET != Get_ExtiInter(9))
    {
        GPIO_EXTI_IRQHandler(9);
    }
    rt_interrupt_leave();
}

void EXTI15_10_IRQHandler(void)
{
    rt_interrupt_enter();
    
    if(RESET != Get_ExtiInter(10))
    {
        GPIO_EXTI_IRQHandler(10);
    }
    if(RESET != Get_ExtiInter(11))
    {
        GPIO_EXTI_IRQHandler(11);
    }
    if(RESET != Get_ExtiInter(12))
    {
        GPIO_EXTI_IRQHandler(12);
    }
    if(RESET != Get_ExtiInter(13))
    {
        GPIO_EXTI_IRQHandler(13);
    }
    if(RESET != Get_ExtiInter(14))
    {
        GPIO_EXTI_IRQHandler(14);
    }
    if(RESET != Get_ExtiInter(15))
    {
        GPIO_EXTI_IRQHandler(15);
    }
    
    rt_interrupt_leave();
}

int rt_hw_pin_init(void)
{
    return rt_device_pin_register("pin", &_at32_pin_ops, RT_NULL);
}

INIT_BOARD_EXPORT(rt_hw_pin_init);

#endif /* RT_USING_PIN */
