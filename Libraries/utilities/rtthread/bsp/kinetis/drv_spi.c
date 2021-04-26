#include <rthw.h>
#include <rtthread.h>
#include "spi.h"
#include "gpio.h"
#include <rtdevice.h>
#include <drivers/spi.h>

static struct rt_spi_bus kinetis_spi;


#define SPI20_PORT      HW_GPIOB
#define SPI20_PIN       20

static rt_err_t configure(struct rt_spi_device* device, struct rt_spi_configuration* configuration)
{
    SPI_Init(SPI2_SCK_PB21_MOSI_PB22_MISO_PB23, 32*1000*1000);
    return RT_EOK;
}

static rt_uint32_t xfer(struct rt_spi_device* device, struct rt_spi_message* message)
{

    rt_uint32_t size = message->length;
    const rt_uint8_t * send_ptr = message->send_buf;
    rt_uint8_t * recv_ptr = message->recv_buf;
    
    rt_uint32_t cs = (rt_uint32_t)(device->parent.user_data);

    if(cs == 0)
    {
        GPIO_PinWrite(SPI20_PORT, SPI20_PIN, 0);
    }
    
    while(size--)
    {
        rt_uint16_t data = 0xFF;
        if(send_ptr != RT_NULL)
        {
            data = *send_ptr++;
        }

        data = SPI_ReadWrite(HW_SPI2, data);
        
        if(recv_ptr != RT_NULL)
        {
            *recv_ptr++ = data;
        }

        if((size == 0) && (message->cs_release))
        {
            if(cs == 0)
            {
                GPIO_PinWrite(SPI20_PORT, SPI20_PIN, 1);
            }
        }
    }
    return message->length;
}

static struct rt_spi_ops kinetis_spi_ops =
{
    configure,
    xfer
};

int rt_hw_spi_bus_init(const char *name)
{
    kinetis_spi.ops = &kinetis_spi_ops;
    return rt_spi_bus_register(&kinetis_spi, name, &kinetis_spi_ops); 
}

void rt_hw_spi_init(const char *spi_bus_name, const char *spi_device_name, uint32_t cs)
{
    struct rt_spi_device* dev;

    dev = rt_malloc(sizeof(struct rt_spi_device));

    if(cs == 0)
    {
        GPIO_Init(SPI20_PORT, SPI20_PIN, kGPIO_OPPH); //SPI2_PCS0
    }

    rt_spi_bus_attach_device(dev, spi_device_name, spi_bus_name, (rt_uint32_t*)cs);
}

INIT_BOARD_EXPORT(rt_hw_spi_init);

