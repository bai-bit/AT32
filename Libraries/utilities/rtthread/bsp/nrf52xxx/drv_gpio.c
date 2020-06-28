#include <rtthread.h>
#include <rtdevice.h>
#include "common.h"
#include "gpio.h"


struct pin_index
{
    uint8_t pin;
    uint8_t hw_index;
    uint8_t  socket;
    void (*hdr)(void *args);
};

static uint8_t socket2pin_tbl[8];

static struct pin_index pins[] = 
{
    {0, 0,   HW_INT_SOCK0, RT_NULL},
    {1, 1,   HW_INT_SOCK1, RT_NULL},
    {2, 2,   HW_INT_SOCK2, RT_NULL},
    {3, 3,   HW_INT_SOCK3, RT_NULL},
    {4, 4,   HW_INT_SOCK4, RT_NULL},
    {5, 5,   HW_INT_SOCK5, RT_NULL},
    {6, 6,   HW_INT_SOCK6, RT_NULL},
    {7, 7,   HW_INT_SOCK7, RT_NULL},
    {8, 8,   HW_INT_SOCK0, RT_NULL},
    {9, 9,   HW_INT_SOCK1, RT_NULL},
    {10, 10, HW_INT_SOCK2, RT_NULL},
    {11, 11, HW_INT_SOCK3, RT_NULL},
    {12, 12, HW_INT_SOCK4, RT_NULL},
    {13, 13, HW_INT_SOCK5, RT_NULL},
    {14, 14, HW_INT_SOCK6, RT_NULL},
    {15, 15, HW_INT_SOCK7, RT_NULL},
    {16, 16, HW_INT_SOCK0, RT_NULL},
    {17, 17, HW_INT_SOCK1, RT_NULL},
    {18, 18, HW_INT_SOCK2, RT_NULL},
    {19, 19, HW_INT_SOCK3, RT_NULL},
    {20, 20, HW_INT_SOCK4, RT_NULL},
    {21, 21, HW_INT_SOCK5, RT_NULL},
    {22, 22, HW_INT_SOCK6, RT_NULL},
    {23, 23, HW_INT_SOCK7, RT_NULL},
    {24, 24, HW_INT_SOCK0, RT_NULL},
    {25, 25, HW_INT_SOCK1, RT_NULL},
    {26, 26, HW_INT_SOCK2, RT_NULL},
    {27, 27, HW_INT_SOCK3, RT_NULL},
    {28, 28, HW_INT_SOCK4, RT_NULL},
    {29, 29, HW_INT_SOCK5, RT_NULL},
    {30, 30, HW_INT_SOCK6, RT_NULL},
    {31, 31, HW_INT_SOCK7, RT_NULL},
};

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

static struct pin_index *get_pin_from_socket(uint32_t socket)
{
    return get_pin(socket2pin_tbl[socket]);
}

void nrf_pin_write(rt_device_t dev, rt_base_t pin, rt_base_t value)
{
//    struct pin_index *p = get_pin(pin)
//    if(p)
//    {
//        GPIO_PinWrite(HW_GPIOA, p->hw_index, value);
//    }
    if(pin < 32)
    {
        GPIO_PinWrite(HW_GPIOA, pin, value);
    }
}

int nrf_pin_read(rt_device_t dev, rt_base_t pin)
{
    int val = 0;
//    struct pin_index *p = get_pin(pin);
//    if(p)
//    {
//        return GPIO_PinRead(HW_GPIOA, p->hw_index);
//    }
//    return 0;
    if(pin < 32)
    {
        val = GPIO_PinRead(HW_GPIOA, pin);
    }
    return val;
}

void nrf_pin_mode(rt_device_t dev, rt_base_t pin, rt_base_t mode)
{
    struct pin_index *p = get_pin(pin);

    switch(mode)
    {
        case PIN_MODE_OUTPUT:
            GPIO_Init(HW_GPIOA, p->hw_index, kGPIO_OPPH);
            break;
        case PIN_MODE_INPUT:
            GPIO_Init(HW_GPIOA, p->hw_index, kGPIO_IFT);
            break;
        case PIN_MODE_INPUT_PULLUP:
            GPIO_Init(HW_GPIOA, p->hw_index, kGPIO_IPU);
            break;
        case PIN_MODE_INPUT_PULLDOWN:
            GPIO_Init(HW_GPIOA, p->hw_index, kGPIO_IPD);
            break;
        case PIN_MODE_OUTPUT_OD:
            GPIO_Init(HW_GPIOA, p->hw_index, kGPIO_OD);
            break;
        default:
            break;
    }
}

rt_err_t nrf_pin_attach_irq(struct rt_device *device, rt_int32_t pin, rt_uint32_t mode, void (*hdr)(void *args), void *args)             
{
    struct pin_index *p = get_pin(pin);

    if(p)
    {
        switch(mode)
        {
            case PIN_IRQ_MODE_RISING:
                NRF_GPIOTE->CONFIG[p->socket] = (p->hw_index<<GPIOTE_CONFIG_PSEL_Pos) | GPIOTE_CONFIG_MODE_Event | (GPIOTE_CONFIG_POLARITY_LoToHi << GPIOTE_CONFIG_POLARITY_Pos);
                break;
            case PIN_IRQ_MODE_FALLING:
                NRF_GPIOTE->CONFIG[p->socket] = (p->hw_index<<GPIOTE_CONFIG_PSEL_Pos) | GPIOTE_CONFIG_MODE_Event | (GPIOTE_CONFIG_POLARITY_HiToLo << GPIOTE_CONFIG_POLARITY_Pos);
                break;
            case PIN_IRQ_MODE_RISING_FALLING:
                break;
        }
        
        p->hdr = hdr;
        socket2pin_tbl[p->socket] = p->pin;
    }

    
    return RT_EOK;
}


rt_err_t nrf_pin_detach_irq(struct rt_device *device, rt_int32_t pin)
{
    struct pin_index *p = get_pin(pin);
    p->hdr = RT_NULL;
    return RT_EOK;
}


rt_err_t nrf_pin_irq_enable(struct rt_device *device, rt_base_t pin, rt_uint32_t enabled)
{
    const struct pin_index *p = get_pin(pin);

    GPIO_SetIntMode(p->socket, enabled);
    return RT_EOK;
}

const static struct rt_pin_ops _nrf_pin_ops =
{
    nrf_pin_mode,
    nrf_pin_write,
    nrf_pin_read,
    nrf_pin_attach_irq,
    nrf_pin_detach_irq,
    nrf_pin_irq_enable,
};


int nrf_hw_pin_init(void)
{
    return rt_device_pin_register("pin", &_nrf_pin_ops, RT_NULL);
}

void GPIOTE_IRQHandler(void)
{
    int i;
    rt_interrupt_enter();
    
    const struct pin_index *p = RT_NULL;
    
    for(i=0; i<8; i++)
    {
        if(NRF_GPIOTE->EVENTS_IN[i] == 1)
        {
            NRF_GPIOTE->EVENTS_IN[i] = 0;

            p = get_pin_from_socket(i);
            if (p && p->hdr)
            {
                p->hdr(RT_NULL);
            }
        }
    }

    rt_interrupt_leave();
}

