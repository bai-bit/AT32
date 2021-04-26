#include "gpio.h"
#include "pin.h"
#include "common.h"
#include "finsh.h"


static void _pin_mode(struct rt_device *device, rt_base_t pin, rt_base_t mode)
{
    uint32_t instance;
    instance = (pin & 0xFF00) >> 8;
    switch(mode)
    {
        case PIN_MODE_OUTPUT:
            GPIO_Init(instance, pin & 0xFF, kGPIO_OPPH);
            break;
        case PIN_MODE_INPUT:
            GPIO_Init(instance, pin & 0xFF, kGPIO_IFT);
            GPIO_SetIntMode(instance, pin & 0xFF, kGPIO_Int_FE, true);
            break;
        case PIN_MODE_INPUT_PULLUP:
            GPIO_Init(instance, pin & 0xFF, kGPIO_IPU);
            GPIO_SetIntMode(instance, pin & 0xFF, kGPIO_Int_FE, true);
            break;
    }
}

static void _pin_write(struct rt_device *device, rt_base_t pin, rt_base_t value)
{
    uint32_t instance;
    instance = (pin & 0xFF00) >> 8;
    GPIO_PinWrite(instance, pin & 0xFF, value);
}

static int _pin_read(struct rt_device *device, rt_base_t pin)
{
    uint32_t instance;
    instance = (pin & 0xFF00) >> 8;
    return GPIO_PinRead(instance, pin & 0xFF);
}
    
static const struct rt_pin_ops _ops =
{
    _pin_mode,
    _pin_write,
    _pin_read,
};

int rt_hw_pin_init(const char *name)
{    
    return rt_device_pin_register(name, &_ops, RT_NULL);
}


void PORTA_IRQHandler(void)
{
    uint32_t flag = PORTA->ISFR;
    PORTA->ISFR = 0xFFFFFFFF;
    
    rt_device_t dev;
    dev = rt_device_find("gpio");
    if(dev && dev->rx_indicate)
    {
        dev->rx_indicate(dev, flag);
    }
}

void PORTB_IRQHandler(void)
{
    uint32_t flag = PORTB->ISFR;
    PORTB->ISFR = 0xFFFFFFFF;
    
    rt_device_t dev;
    dev = rt_device_find("gpio");
    if(dev && dev->rx_indicate)
    {
        dev->rx_indicate(dev, flag);
    }
}

void PORTC_IRQHandler(void)
{
    uint32_t flag = PORTC->ISFR;
    PORTC->ISFR = 0xFFFFFFFF;
    
    rt_device_t dev;
    dev = rt_device_find("gpio");
    if(dev && dev->rx_indicate)
    {
        dev->rx_indicate(dev, flag);
    }
}

void PORTD_IRQHandler(void)
{
    uint32_t flag = PORTD->ISFR;
    PORTD->ISFR = 0xFFFFFFFF;
    
    rt_device_t dev;
    dev = rt_device_find("gpio");
    if(dev && dev->rx_indicate)
    {
        dev->rx_indicate(dev, flag);
    }
}

void PORTE_IRQHandler(void)
{
    uint32_t flag = PORTE->ISFR;
    PORTE->ISFR = 0xFFFFFFFF;
    
    rt_device_t dev;
    dev = rt_device_find("gpio");
    if(dev && dev->rx_indicate)
    {
        dev->rx_indicate(dev, flag);
    }
}

