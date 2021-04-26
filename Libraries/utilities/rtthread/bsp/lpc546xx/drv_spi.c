#include <rthw.h>
#include <stdio.h>
#include <rtthread.h>
#include "common.h"
#include "spi.h"
#include "gpio.h"
#include <rtdevice.h>
#include <drivers/spi.h>

static struct rt_spi_bus bus;

#define SPI10_CS        P1out(26)
#define SPI21_CS        P1out(0)

struct lpc_spi_device
{
    struct rt_spi_device       rt_spi_dev;
    uint8_t                    bus_instance;
    uint8_t                    cs_port;
    uint8_t                    cs_pin;
    uint8_t                    cs;
};


static rt_err_t configure(struct rt_spi_device* device, struct rt_spi_configuration* configuration)
{
    return RT_EOK;
}

static rt_uint32_t xfer(struct rt_spi_device* device, struct rt_spi_message* message)
{
    rt_uint32_t size = message->length;
    const rt_uint8_t * send_ptr = message->send_buf;
    rt_uint8_t * recv_ptr = message->recv_buf;

    struct lpc_spi_device* dev = (struct lpc_spi_device*)device->parent.user_data;
    
    /* take cs */
    switch(dev->cs)
    {
        case 0:
            SPI10_CS = 0;
            break;
        case 1:
            SPI21_CS = 0;
            break;
    }

    
    SPI_ReadWriteFIFO(dev->bus_instance, recv_ptr, (uint8_t*)send_ptr, size);
    
    if(message->cs_release)
    {
        switch(dev->cs)
        {
            case 0:
                SPI10_CS = 1;
                break;
            case 1:
                SPI21_CS = 1;
                break;
        }
    }
    return message->length;
}

static struct rt_spi_ops ops =
{
    configure,
    xfer
};

int rt_hw_spi_bus_init(const char *name)
{
    uint32_t instance;
    
    sscanf(name, "spi%d", &instance);
    
    SPI_Init(instance, 1000*1000*48);
    switch(instance)
    {
        case 1:
            SetPinMux(HW_GPIO0, 29, 1);
            SetPinMux(HW_GPIO0, 30, 1);
            SetPinMux(HW_GPIO1, 10, 4); /* SCK */
            break;
        case 2:
//            SetPinMux(HW_GPIO1, 23, 1);
//            SetPinMux(HW_GPIO1, 24, 1);
//            SetPinMux(HW_GPIO1, 25, 1);
//            SetPinMux(HW_GPIO0, 8, 1);
//            SetPinMux(HW_GPIO0, 9, 1);
//            SetPinMux(HW_GPIO0, 10, 1);
            break;
    }

    return rt_spi_bus_register(&bus, name, &ops); 
}

void rt_hw_spi_init(const char *spi_bus_name, const char *spi_device_name, uint32_t cs)
{
    struct lpc_spi_device* lpc_spi_dev = rt_malloc(sizeof(struct lpc_spi_device));
    
    sscanf(spi_bus_name, "spi%d", (int*)&lpc_spi_dev->bus_instance);
    lpc_spi_dev->cs = cs;
    
    switch(lpc_spi_dev->cs)
    {
        case 0:
            switch(lpc_spi_dev->bus_instance)
            {
                case 2:
                    GPIO_Init(HW_GPIO1, 26, kGPIO_OPPH);
                    lpc_spi_dev->cs_port = HW_GPIO1;
                    lpc_spi_dev->cs_pin = 26;
                    break;
            }
            break;
        case 1:
            GPIO_Init(HW_GPIO1, 0, kGPIO_OPPH);
            lpc_spi_dev->cs_port = HW_GPIO1;
            lpc_spi_dev->cs_pin = 0;
            break;
    }
    
    rt_spi_bus_attach_device(&lpc_spi_dev->rt_spi_dev, spi_device_name, spi_bus_name, (void*)lpc_spi_dev);
}

INIT_BOARD_EXPORT(rt_hw_spi_init);

