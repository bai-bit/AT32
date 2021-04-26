#include <rthw.h>
#include <rtthread.h>
#include "common.h"
#include "gpio.h"
#include <rtdevice.h>
#include <drivers/pin.h>


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
            GPIO_AddIntToSocket(instance, pin & 0xFF, 0);
            GPIO_SetIntMode(0, kGPIO_Int_FE, true);
            break;
        case PIN_MODE_INPUT_PULLUP:
            GPIO_Init(instance, pin & 0xFF, kGPIO_IPU);
            GPIO_AddIntToSocket(instance, pin & 0xFF, 0);
            GPIO_SetIntMode(0, kGPIO_Int_FE, true);
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

void PIN_INT0_IRQHandler(void)
{
    volatile uint32_t IST = PINT->IST;
    PINT->IST = IST;
    
    rt_device_t dev;
    dev = rt_device_find("gpio");
    if(dev && dev->rx_indicate)
    {
        dev->rx_indicate(dev, IST);
    }
}

INIT_BOARD_EXPORT(rt_hw_pin_init);
