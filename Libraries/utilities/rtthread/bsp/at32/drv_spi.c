
#include <rtthread.h>
#include <stdio.h>
#include <rtdevice.h>
#include "common.h"
#include "spi.h"
#include "dma.h"
#include "gpio.h"


#define AT32_SPI_CNT        (4)

struct rt_spi_bus spi_bus[AT32_SPI_CNT];
//rt_sem_t sem_spi_dma;

static volatile uint8_t ready = 0;

typedef struct
{
    uint8_t cs_pin;         /* cs pin */
    uint8_t instance;       /* spi instance */
    uint8_t dma_tx_chl;     /* spi tx dma channel */
    uint8_t dma_rx_chl;     /* spi rx dma channel */
    uint8_t dma_instance;   /* dma instance */
}spi_usr_t;


static rt_err_t configure(struct rt_spi_device* device, struct rt_spi_configuration* configuration)
{
    spi_usr_t *usr = device->parent.user_data;
    switch(usr->instance)
    {
        case HW_SPI1:
            SPI_Init(HW_SPI1, 8*1000*1000);
            SPI_SetMode(HW_SPI1, 1, 1);
            SPI_DMAInit(HW_SPI1, usr->dma_instance, usr->dma_tx_chl, usr->dma_rx_chl);
            // DMA_SetChlIntMode(usr->dma_rx_chl, 1);
            break;
        case HW_SPI3:
            SPI_Init(HW_SPI3, configuration->max_hz);
            DMA_Init(usr->dma_instance);
            SPI_DMAInit(HW_SPI3, usr->dma_instance, usr->dma_tx_chl, usr->dma_rx_chl);
            break;
    }

    return RT_EOK;
}

static rt_uint32_t xfer(struct rt_spi_device* device, struct rt_spi_message* message)
{
    rt_uint32_t size = message->length;
    rt_uint8_t *send_ptr = (rt_uint8_t*)message->send_buf;
    rt_uint8_t *recv_ptr = message->recv_buf;
    spi_usr_t *usr = device->parent.user_data;

    static uint8_t dummy_buf[1];
    
    rt_pin_write(usr->cs_pin, 0);
    
    if(send_ptr == RT_NULL)
    {
        DMA_SetMemInc(usr->dma_instance, usr->dma_tx_chl, 0);
        DMA_SetMemInc(usr->dma_instance, usr->dma_rx_chl, 1);
        SPI_StartDMATxRx(usr->instance, usr->dma_instance, dummy_buf, recv_ptr, usr->dma_tx_chl, usr->dma_rx_chl, size);
    }
    else if(recv_ptr == RT_NULL)
    {
        DMA_SetMemInc(usr->dma_instance, usr->dma_tx_chl, 1);
        DMA_SetMemInc(usr->dma_instance, usr->dma_rx_chl, 0);
        SPI_StartDMATxRx(usr->instance, usr->dma_instance, send_ptr, dummy_buf, usr->dma_tx_chl, usr->dma_rx_chl, size);
    }
    else
    {
        DMA_SetMemInc(usr->dma_instance, usr->dma_tx_chl, 1);
        DMA_SetMemInc(usr->dma_instance, usr->dma_rx_chl, 1);
        SPI_StartDMATxRx(usr->instance, usr->dma_instance, send_ptr, recv_ptr, usr->dma_tx_chl, usr->dma_rx_chl, size);
    }
    
   while(DMA_GetTransferCnt(usr->dma_instance, usr->dma_rx_chl) != 0) {};

//    rt_sem_take(sem_spi_dma, RT_WAITING_FOREVER);

    if(message->cs_release)
    {
        rt_pin_write(usr->cs_pin, 1);
    }
    
//    while(size--)
//    {
//        rt_uint16_t data = 0xFF;
//        if(send_ptr != RT_NULL)
//        {
//            data = *send_ptr++;
//        }

//        if((size == 0) && (message->cs_release))
//        {  
//            data = SPI_ReadWrite(usr->instance, data);
//            rt_pin_write(usr->cs_pin, 1);
//        }
//        else
//        {
//            data = SPI_ReadWrite(usr->instance, data);
//        }
//        if(recv_ptr != RT_NULL)
//        {
//            *recv_ptr++ = data;
//        }
//    }

    
    return message->length;
}


static const struct rt_spi_ops ops =
{
    configure,
    xfer
};


int rt_hw_spi_init(const char *name)
{
    uint32_t instance;
    sscanf(name, "spi%d", &instance);
    return rt_spi_bus_register(&spi_bus[instance], name, &ops); 
}


int rt_hw_spi_device_init(const char *name, const char *bus, rt_uint32_t cs_pin)
{
    uint32_t instance;
    sscanf(bus, "spi%d", &instance);
    
    struct rt_spi_device *dev = rt_malloc(sizeof(struct rt_spi_device));
    spi_usr_t *usr = rt_malloc(sizeof(spi_usr_t));
    
    usr->instance = instance;
    usr->cs_pin = cs_pin;
    
    if(instance == HW_SPI1)
    {
        usr->dma_instance = HW_DMA1;
        usr->dma_tx_chl = HW_DMA_CH3;
        usr->dma_rx_chl = HW_DMA_CH2;
        SetPinMux(HW_GPIOA, 5, 2, 2);   /* CLK */
        SetPinMux(HW_GPIOA, 6, 1, 0);   /* MISO */
        SetPinMux(HW_GPIOA, 7, 2, 2);   /* MOSI */
    }
    
    if(instance == HW_SPI2)
    {
        usr->dma_instance = HW_DMA1;
        usr->dma_tx_chl = HW_DMA_CH5;
        usr->dma_rx_chl = HW_DMA_CH4;
    }
    
    if(instance == HW_SPI3)
    {
        usr->dma_instance = HW_DMA2;
        usr->dma_tx_chl = HW_DMA_CH2;
        usr->dma_rx_chl = HW_DMA_CH1;
        SetPinMux(HW_GPIOB,  3, 2, 3);   /* CLK */
        SetPinMux(HW_GPIOB,  4, 1, 0);   /* MISO */
        SetPinMux(HW_GPIOB,  5, 2, 3);   /* MOSI */
    }
    
    /* set spi cs pin to output mode */
    rt_pin_mode(cs_pin, PIN_MODE_OUTPUT);
    
    return rt_spi_bus_attach_device(dev, name, bus, usr);
}


void DMA1_Channel2_IRQHandler(void)
{
    rt_interrupt_enter();
    
    DMA1->ICLR = DMA_ICLR_CTCIF2 | DMA_ICLR_CGIF2 | DMA_ICLR_CHTIF2 | DMA_ICLR_CERRIF2;

    rt_interrupt_leave();
}
