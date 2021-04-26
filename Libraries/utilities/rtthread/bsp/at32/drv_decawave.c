#include <rtthread.h>
#include <stdio.h>
#include <rtdevice.h>

#include "deca_types.h"
#include "deca_device_api.h"


#define DW_DEBUG		1

#if ( DW_DEBUG == 1 )
#include <stdio.h>
#define DW_TRACE	printf
#else
#define DW_TRACE(...)
#endif

typedef struct
{
    struct                  rt_device parent;
    struct rt_spi_device    *spi_dev;
    uint8_t                 irq_pin;
    uint8_t                 rst_pin;
}rt_dw1000_t;

static rt_dw1000_t rt_dw1000;
static void dw1000_irq_handler(void *args);


/* decaware port function */
void deca_sleep(unsigned int time_ms)
{
    rt_thread_delay(time_ms);
}

#include "common.h"
#include "spi.h"
#include "gpio.h"
#include "dma.h"

/* decaware port function */
int writetospi(uint16_t headerLength,  const uint8_t *headerBuffer, uint32_t bodyLength, const uint8_t *bodyBuffer)           
{
    PAout(15) = 0;
    if(bodyLength < 8)
    {
        while(headerLength--)
        {
            SPI3->DT = *headerBuffer++;
            while(SPI3->STS & SPI_STS_BSY) {};
        }
        while(bodyLength--)
        {
            SPI3->DT = *bodyBuffer++;
            while(SPI3->STS & SPI_STS_BSY) {};
        }
    }
    else
    {
        SPI_StartDMATxRx(HW_SPI3, HW_DMA2, (uint8_t*)headerBuffer, NULL, HW_DMA_CH2, HW_DMA_CH1, headerLength);
        while(DMA_GetTransferCnt(HW_DMA2, HW_DMA_CH1) != 0) {};
        SPI_StartDMATxRx(HW_SPI3, HW_DMA2, (uint8_t*)bodyBuffer, NULL, HW_DMA_CH2, HW_DMA_CH1, bodyLength);
        while(DMA_GetTransferCnt(HW_DMA2, HW_DMA_CH1) != 0) {};
    }

    PAout(15) = 1;
  //  rt_spi_send_then_send(rt_dw1000.spi_dev, headerBuffer, headerLength, bodyBuffer, bodyLength);
    return 0;
}

/* decaware port function */
int readfromspi(uint16_t headerLength, const uint8_t *headerBuffer, uint32_t readlength, uint8_t *readBuffer)          
{
    static uint8_t dummy;
    PAout(15) = 0;
    
    if(readlength < 8)
    {
        while(headerLength--)
        {
            SPI3->DT = *headerBuffer++;
            while(SPI3->STS & SPI_STS_BSY) {};
            dummy = SPI3->DT;
        }
        while(readlength--)
        {
            SPI3->DT = dummy;
            while(SPI3->STS & SPI_STS_BSY) {};
            *readBuffer++ = SPI3->DT;
        }
    }
    else
    {
        SPI_StartDMATxRx(HW_SPI3, HW_DMA2, (uint8_t*)headerBuffer, NULL, HW_DMA_CH2, HW_DMA_CH1, headerLength);
        while(DMA_GetTransferCnt(HW_DMA2, HW_DMA_CH1) != 0) {};
        SPI_StartDMATxRx(HW_SPI3, HW_DMA2, NULL, (uint8_t*)readBuffer, HW_DMA_CH2, HW_DMA_CH1, readlength);
        while(DMA_GetTransferCnt(HW_DMA2, HW_DMA_CH1) != 0) {};
    }
        

    PAout(15) = 1;
    
 //   rt_spi_send_then_recv(rt_dw1000.spi_dev, headerBuffer, headerLength, readBuffer, readlength);
    return 0;
}


static rt_err_t rt_dw1000_init(rt_device_t dev)
{    
    return RT_EOK;
}


/* Default communication configuration. We use here EVK1000's default mode (mode 3). */
static dwt_config_t config = {
    2,               /* Channel number. */
    DWT_PRF_64M,     /* Pulse repetition frequency. */
    DWT_PLEN_128,    /* Preamble length. Used in TX only. */
    DWT_PAC8,        /* Preamble acquisition chunk size. Used in RX only. */
    9,               /* TX preamble code. Used in TX only. */
    9,               /* RX preamble code. Used in RX only. */
    1,               /* 0 to use standard SFD, 1 to use non-standard SFD. */
    DWT_BR_6M8,      /* Data rate. */
    DWT_PHRMODE_STD, /* PHY header mode. */
    (129 + 8 - 8)    /* SFD timeout (preamble length + 1 + SFD length - PAC size). Used in RX only. */
};

static rt_err_t rt_dw1000_open(rt_device_t dev, rt_uint16_t oflag)
{
    uint32_t ret;
    
    /* hardware reset */
    rt_pin_mode(rt_dw1000.rst_pin, PIN_MODE_OUTPUT);
    
    rt_pin_write(rt_dw1000.rst_pin, 0);
    rt_thread_delay(rt_tick_from_millisecond(1));
    rt_pin_write(rt_dw1000.rst_pin, 1);
    rt_thread_delay(rt_tick_from_millisecond(2));
    
    struct rt_spi_configuration cfg = {0};
    
    cfg.max_hz = 1*1000*1000;
    rt_spi_configure(rt_dw1000.spi_dev, &cfg);

    ret = dwt_initialise(DWT_LOADUCODE);

    DW_TRACE("%-32s:0x%X\r\n", "dwt_initialise", ret);
    if(ret)
    {
        DW_TRACE("wdt init failed!\r\n");
        return RT_EIO;
    }
    
//    cfg.max_hz = 16*1000*1000;
//    rt_spi_configure(rt_dw1000.spi_dev, &cfg);
//    
    dwt_configure(&config);
    
    void _dwt_configlde(int prfIndex);
    _dwt_configlde(1);
    
    dwt_enablegpioclocks();
    dwt_setlnapamode(DWT_LNA_ENABLE | DWT_PA_ENABLE);
    dwt_setfinegraintxseq(0);
    dwt_configeventcounters(1);
    dwt_setsmarttxpower(0);
    dwt_setleds(DWT_LEDS_ENABLE);

    /* channel2 tx power */
    dwt_txconfig_t tcrfcfg;
    tcrfcfg.PGdly = 0xC2; //PG_DELAY
    tcrfcfg.power = 0x67676767;//64M prf power
    dwt_configuretxrf(&tcrfcfg);
    
    rt_thread_delay(1);
        
    SPI_Init(HW_SPI3, 20*1000*1000);
    DMA_Init(HW_DMA2);
    SPI_DMAInit(HW_SPI3, HW_DMA2, HW_DMA_CH2, HW_DMA_CH1);
    
    /* install irq */
    rt_pin_mode(rt_dw1000.irq_pin, PIN_MODE_INPUT);
    rt_pin_attach_irq(rt_dw1000.irq_pin, PIN_IRQ_MODE_RISING, dw1000_irq_handler, RT_NULL);
    rt_pin_irq_enable(rt_dw1000.irq_pin, PIN_IRQ_ENABLE);
    
    return RT_EOK;
}


static rt_size_t rt_dw1000_read(rt_device_t dev, rt_off_t pos, void* buffer, rt_size_t size)
{
    return size;
}


static rt_err_t rt_dw1000_control(rt_device_t dev, int cmd, void *args)
{
    return RT_EOK;
}


int rt_hw_dw1000_init(const char *name, const char *spid_name, uint32_t rst_pin, uint32_t irq_pin)
{
    rt_dw1000.spi_dev =  (struct rt_spi_device *)rt_device_find(spid_name);
    
    struct rt_device *dev;
    
    dev                     = &rt_dw1000.parent;
	dev->type               = RT_Device_Class_Miscellaneous;
	dev->rx_indicate        = RT_NULL;
	dev->tx_complete        = RT_NULL;
	dev->init               = rt_dw1000_init;
	dev->open               = rt_dw1000_open;
	dev->close              = RT_NULL;
	dev->read               = rt_dw1000_read;
	dev->write              = RT_NULL;
	dev->control            = rt_dw1000_control;
	dev->user_data          = &rt_dw1000;
 
    /* install irq pins */
    rt_dw1000.irq_pin = irq_pin;
    rt_dw1000.rst_pin = rst_pin;
    
    rt_device_register(dev, name, RT_DEVICE_FLAG_RDWR);
    
    SPI_Init(HW_SPI3, 1*1000*1000);
    DMA_Init(HW_DMA2);
    SPI_DMAInit(HW_SPI3, HW_DMA2, HW_DMA_CH2, HW_DMA_CH1);
    return RT_EOK; 
}


static void dw1000_irq_handler(void *args)
{     
    dwt_isr();
}


decaIrqStatus_t decamutexon(void)           
{
    rt_enter_critical();
    return 0;
}

void decamutexoff(decaIrqStatus_t s)        // put a function here that re-enables the interrupt at the end of the critical section
{
    rt_exit_critical();
}



