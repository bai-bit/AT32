#include <rtdevice.h>
#include "common.h"
#include "gpio.h"

/* indicate which port in attached to interrupt line */
static uint8_t gpio_pin2port[16];

struct pin_index
{
    uint8_t             pin;
    uint8_t             hw_port;
    uint8_t             hw_pin;
    void                (*hdr)(void *args);
};

static struct pin_index pins[] = 
{
    {0, HW_GPIOA,  0, RT_NULL},
    {1, HW_GPIOA,  1, RT_NULL},
    {2, HW_GPIOA,  2, RT_NULL},
    {3, HW_GPIOA,  3, RT_NULL},
    {4, HW_GPIOA,  4, RT_NULL},
    {5, HW_GPIOA,  5, RT_NULL},
    {6, HW_GPIOA,  6, RT_NULL},
    {7, HW_GPIOA,  7, RT_NULL},
    {8, HW_GPIOA,  8, RT_NULL},
    {9, HW_GPIOA,  9, RT_NULL},
    {10, HW_GPIOA, 10, RT_NULL},
    {11, HW_GPIOA, 11, RT_NULL},
    {12, HW_GPIOA, 12, RT_NULL},
    {13, HW_GPIOA, 13, RT_NULL},
    {14, HW_GPIOA, 14, RT_NULL},
    {15, HW_GPIOA, 15, RT_NULL},
    
    {16, HW_GPIOB, 0, RT_NULL},
    {17, HW_GPIOB, 1, RT_NULL},
    {18, HW_GPIOB, 2, RT_NULL},
    {19, HW_GPIOB, 3, RT_NULL},
    {20, HW_GPIOB, 4, RT_NULL},
    {21, HW_GPIOB, 5, RT_NULL},
    {22, HW_GPIOB, 6, RT_NULL},
    {23, HW_GPIOB, 7, RT_NULL},
    {24, HW_GPIOB, 8, RT_NULL},
    {25, HW_GPIOB, 9, RT_NULL},
    {26, HW_GPIOB, 10, RT_NULL},
    {27, HW_GPIOB, 11, RT_NULL},
    {28, HW_GPIOB, 12, RT_NULL},
    {29, HW_GPIOB, 13, RT_NULL},
    {30, HW_GPIOB, 14, RT_NULL},
    {31, HW_GPIOB, 15, RT_NULL},
};

static struct pin_index *get_pin_from_hw(uint32_t port, uint32_t pin)
{
    int i;
    for(i=0; i<ARRAY_SIZE(pins); i++)
    {
        if((port == pins[i].hw_port) && (pin == pins[i].hw_pin))
        {
            return &pins[i];
        }
    }
    return RT_NULL; 
}

static struct pin_index *get_pin(uint32_t pin)
{
    int i;
    for(i=0; i<ARRAY_SIZE(pins); i++)
    {
        if(pin == pins[i].pin)
        {
            return &pins[i];
        }
    }
    return RT_NULL;
}

static void at32_pin_write(rt_device_t dev, rt_base_t pin, rt_base_t value)
{
//    struct pin_index *p = get_pin(pin);
    if(pin < ARRAY_SIZE(pins))
    {
        GPIO_PinWrite(pin/16, (pin%16), value);
    }
//    if(p)
//    {
//        GPIO_PinWrite(p->hw_port, p->hw_pin, value);
//    }
}

static int at32_pin_read(rt_device_t dev, rt_base_t pin)
{
//    struct pin_index *p = get_pin(pin);
//    if(p)
//    {
//        return GPIO_PinRead(p->hw_port, p->hw_pin);
//    }
    if(pin < ARRAY_SIZE(pins))
    {
        return GPIO_PinRead(pin/16, (pin%16));
    }
    return 0;
}

static void at32_pin_mode(rt_device_t dev, rt_base_t pin, rt_base_t mode)
{
    struct pin_index *p = get_pin(pin);
    if(p)
    {
        switch(mode)
        {
            case PIN_MODE_OUTPUT:
                GPIO_Init(p->hw_port, p->hw_pin, kGPIO_OPPH);
                break;
            case PIN_MODE_INPUT:
                GPIO_Init(p->hw_port, p->hw_pin, kGPIO_IFT);
                break;
            case PIN_MODE_INPUT_PULLUP:
                GPIO_Init(p->hw_port, p->hw_pin, kGPIO_IPU);
                GPIO_SetPinPull(p->hw_port, p->hw_pin, 1);
                break;
            case PIN_MODE_INPUT_PULLDOWN:
                GPIO_Init(p->hw_port, p->hw_pin, kGPIO_IPD);
                GPIO_SetPinPull(p->hw_port, p->hw_pin, 0);
                break;
            case PIN_MODE_OUTPUT_OD:
                GPIO_Init(p->hw_port, p->hw_pin, kGPIO_OD);
                break;
            default:
                break;
        }
    }
}

static rt_err_t at32_pin_attach_irq(struct rt_device *device, rt_int32_t pin, rt_uint32_t mode, void (*hdr)(void *args), void *args)             
{
    struct pin_index *p = get_pin(pin);
    
    switch(mode)
    {
        case PIN_IRQ_MODE_RISING:
            break;
        case PIN_IRQ_MODE_FALLING:
            break;
        case PIN_IRQ_MODE_RISING_FALLING:
            break;
    }
    
    if(p)
    {
        p->hdr = hdr;
        GPIO_SetExitLine(p->hw_port, p->hw_pin);
        gpio_pin2port[p->hw_pin] = p->hw_port;
    }

    return RT_EOK;
}

static rt_err_t at32_pin_detach_irq(struct rt_device *device, rt_int32_t pin)
{
    struct pin_index *p = get_pin(pin);
    if(p)
    {
        p->hdr = RT_NULL;
    }

    return RT_EOK;
}


static rt_err_t at32_pin_irq_enable(struct rt_device *device, rt_base_t pin, rt_uint32_t enabled)
{
    struct pin_index *p = get_pin(pin);
    if(enabled)
    {
        EXIT_SetIntMode(p->hw_port, p->hw_pin, kGPIO_Int_RE, true);
    }
    else
    {
        EXIT_SetIntMode(p->hw_port, p->hw_pin, kGPIO_Int_RE, false);
    }
 
    return RT_EOK;
}

const static struct rt_pin_ops _at32_pin_ops =
{
    at32_pin_mode,
    at32_pin_write,
    at32_pin_read,
    at32_pin_attach_irq,
    at32_pin_detach_irq,
    at32_pin_irq_enable,
};


int rt_hw_pin_init(const char *name)
{
    int result;

    result = rt_device_pin_register(name, &_at32_pin_ops, RT_NULL);
    return result;
}

static void port_irq_handler(uint32_t port, uint32_t pin)
{
    rt_interrupt_enter();
    EXTI->PND = (1<<pin);
    
    struct pin_index *p;

    p = get_pin_from_hw(port, pin);
    if (p && p->hdr)
    {
        p->hdr(RT_NULL);
    }
    rt_interrupt_leave();
}

void EXTI0_IRQHandler(void)
{
    port_irq_handler(gpio_pin2port[0], 0);
}

void EXTI1_IRQHandler(void)
{
    port_irq_handler(gpio_pin2port[1], 1);
}

void EXTI4_IRQHandler(void)
{
    port_irq_handler(gpio_pin2port[4], 4);
}


void EXTI9_5_IRQHandler(void)
{
    for(int i=5; i<10; i++)
    {
        if(EXTI->PND & (1<<i))
        {
            port_irq_handler(gpio_pin2port[i], i);
        }
    }
}

void EXTI15_10_IRQHandler(void)
{
    for(int i=10; i<16; i++)
    {
        if(EXTI->PND & (1<<i))
        {
            port_irq_handler(gpio_pin2port[i], i);
        }
    }
}


