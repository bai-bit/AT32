#include <rtthread.h>
#include <stdio.h>
#include <rtdevice.h>
#include "common.h"
#include "icm20608.h"
#include "spi.h"
#include "gpio.h"

#include "drivers/rt_sensor.h"



static const uint32_t acc_rg_tbl[] = {2, 4, 8, 16};
static const uint32_t acc_bw_tbl[] = {10, 21, 44, 99, 218, 420};
static const uint32_t gyr_rg_tbl[] = {250, 500, 1000, 2000};
static const uint32_t gyr_bw_tbl[] = {5, 10, 20, 41, 92, 176, 250, 3281};

static void icm_pin_int(void *args);


typedef struct
{
    struct rt_device parent;
    uint32_t acc_rg;
    uint32_t acc_bw;
    uint32_t gyr_rg;
    uint32_t gyr_bw;
    uint32_t int_pin;
}rt_icm_t;

static rt_icm_t rt_icm;

/* icm20608 adapter */
static uint32_t icm_write_reg(uint8_t addr, uint8_t *buf, uint32_t len)
{
    GPIO_PinWrite(HW_GPIOB, 0, 0);
    SPI_ReadWrite(HW_SPI1, addr | (0<<7));
    while(len--)
    {
        SPI_ReadWrite(HW_SPI1, *buf++);
    }
    GPIO_PinWrite(HW_GPIOB, 0, 1);
    return CH_OK;
}

static uint32_t icm_read_reg(uint8_t addr, uint8_t *buf, uint32_t len)
{
    GPIO_PinWrite(HW_GPIOB, 0, 0);
    SPI_ReadWrite(HW_SPI1, addr | (1<<7));
    while(len--)
    {
        *buf++ = SPI_ReadWrite(HW_SPI1, 0xFF);
    }
    GPIO_PinWrite(HW_GPIOB, 0, 1);
    return CH_OK;
}

static void delay_ms(uint32_t ms)
{
    rt_thread_delay(rt_tick_from_millisecond(ms));
}

static struct icm_init_t icm_ops = 
{
    icm_write_reg,
    icm_read_reg,
    delay_ms,
};

static rt_err_t rt_icm_init (rt_device_t dev)
{    
    GPIO_Init(HW_GPIOB, 0,  kGPIO_OPPH); /* ACC CS 0xFA */
	
    /* pull the SCK MOSI MISO to default state: bug on AT32 */
    GPIO_Init(HW_GPIOA, 5, kGPIO_OPPL); /* SCK */
    GPIO_Init(HW_GPIOA, 6, kGPIO_OPPL); /* MISO */
    GPIO_Init(HW_GPIOA, 7, kGPIO_OPPL); /* MOSI */
	DelayUs(1000);
    
    SPI_Init(HW_SPI1, 8*1000*1000);
    SetPinMux(HW_GPIOA,  5, 2, 2);   /* CLK */
    SetPinMux(HW_GPIOA,  6, 1, 0);   /* MISO */
    SetPinMux(HW_GPIOA,  7, 2, 2);   /* MOSI */
    
    return RT_EOK;
}

static rt_err_t rt_icm_open(rt_device_t dev, rt_uint16_t oflag)
{
    rt_err_t ret;
    
    ret = icm_init(&icm_ops);
    if(ret == CH_OK)
    {
        icm_enable_raw_data_int(true);
        icm_set_odr(0);
        ret = RT_EOK;
    }
    else
    {
        ret = RT_EIO;
    }
    
    /* enable raw sensor gyr interrupt */
    rt_pin_mode(rt_icm.int_pin, PIN_MODE_INPUT_PULLUP);
    rt_pin_attach_irq(rt_icm.int_pin, PIN_IRQ_MODE_RISING, icm_pin_int, RT_NULL);
    rt_pin_irq_enable(rt_icm.int_pin, PIN_IRQ_ENABLE);
    
    return ret;
}


static rt_size_t rt_icm_read(rt_device_t dev, rt_off_t pos, void* buffer, rt_size_t size)
{
    switch(pos)
    {
        case RT_SENSOR_POS_ACC:
            icm_read_acc(buffer);
            break;
        case RT_SENSOR_POS_ACC_FIFO:
            icm_read_acc(buffer);
            break;
        case RT_SENSOR_POS_GYR:
            icm_read_gyr(buffer);
            break;
        case RT_SENSOR_POS_GYR_FIFO:
            icm_read_gyr(buffer);
            break;
        case RT_SENSOR_POS_GYR_TEMPERATURE:
            icm_read_temperature(buffer);
            break;
        case RT_SENSOR_POS_ACC_BW:
            rt_memcpy(buffer, &acc_bw_tbl[rt_icm.acc_bw], 4);
            break;
        case RT_SENSOR_POS_ACC_RG:
            rt_memcpy(buffer, &acc_rg_tbl[rt_icm.acc_rg], 4);
            break;
        case RT_SENSOR_POS_GYR_RG:
            rt_memcpy(buffer, &gyr_rg_tbl[rt_icm.gyr_rg], 4);
            break;
        case RT_SENSOR_POS_GYR_BW:
            rt_memcpy(buffer, &gyr_bw_tbl[rt_icm.gyr_bw], 4);
            break;
    }
    return size;
}

static rt_size_t rt_icm_write(rt_device_t dev, rt_off_t pos, const void* buffer, rt_size_t size)
{   
    uint8_t *val = (uint8_t *)buffer;
    
    switch(pos)
    {
        case RT_SENSOR_POS_ACC_RG:
            rt_icm.acc_rg = *val;
            switch(*val)
            {
                case 0:
                    icm_set_acc_range(kICM_2G);
                    break;
                case 1:
                    icm_set_acc_range(kICM_4G);
                    break;
                case 2:
                    icm_set_acc_range(kICM_8G);
                    break;
                case 3:
                    icm_set_acc_range(kICM_16G);
                    break;
            }
            break;
        case RT_SENSOR_POS_GYR_RG:
            rt_icm.gyr_rg = *val;
            switch(*val)
            {
                case 0:
                    icm_set_gyr_range(kICM_250DPS);
                    break;
                case 1:
                    icm_set_gyr_range(kICM_500DPS);
                    break;
                case 2:
                    icm_set_gyr_range(kICM_1000DPS);
                    break;
                case 3:
                    icm_set_gyr_range(kICM_2000DPS);
                    break;
            }
            break;
        case RT_SENSOR_POS_ACC_BW:
            rt_icm.acc_bw = *val;
            switch(*val)
            {
                case 0:
                    icm_set_acc_bw(kICM_ACC_BW_10Hz);
                    break;
                case 1:
                    icm_set_acc_bw(kICM_ACC_BW_21Hz);
                    break;
                case 2:
                    icm_set_acc_bw(kICM_ACC_BW_44Hz);
                    break;
                case 3:
                    icm_set_acc_bw(kICM_ACC_BW_99Hz);
                    break;
                case 4:
                    icm_set_acc_bw(kICM_ACC_BW_218Hz);
                    break;
                case 5:
                    icm_set_acc_bw(kICM_ACC_BW_420Hz);
                    break;
                default:
                    icm_set_acc_bw(kICM_ACC_BW_420Hz);
                    break;
            }
            break;
        case RT_SENSOR_POS_GYR_BW:
            rt_icm.gyr_bw = *val;
            switch(*val)
            {
                case 0:
                    icm_set_gyr_bw(kICM_5HZ);
                    break;
                case 1:
                    icm_set_gyr_bw(kICM_10HZ);
                    break;
                case 2:
                    icm_set_gyr_bw(kICM_20HZ);
                    break;
                case 3:
                    icm_set_gyr_bw(kICM_41HZ);
                    break;
                case 4:
                    icm_set_gyr_bw(kICM_92HZ);
                    break;
                case 5:
                    icm_set_gyr_bw(kICM_176HZ);
                    break;
                case 6:
                    icm_set_gyr_bw(kICM_250HZ);
                    break;
                default:
                    icm_set_gyr_bw(kICM_3281Hz);
                    break;
            }
            break;
    }
    return size;
}

static rt_err_t  rt_icm_control(rt_device_t dev, int cmd, void *args)
{
    return RT_EOK;
}

int rt_hw_icm20602_init(const char *name, uint32_t int_pin)
{
    struct rt_device *dev;
    
    dev                     = &rt_icm.parent;
	dev->type               = RT_Device_Class_Miscellaneous;
	dev->rx_indicate        = RT_NULL;
	dev->tx_complete        = RT_NULL;
	dev->init               = rt_icm_init;
	dev->open               = rt_icm_open;
	dev->close              = RT_NULL;
	dev->read               = rt_icm_read;
	dev->write              = rt_icm_write;
	dev->control            = rt_icm_control;
	dev->user_data          = RT_NULL;
 
    rt_icm.int_pin = int_pin;
    
    
    rt_device_register(dev, name, RT_DEVICE_FLAG_RDWR);
    return RT_EOK;
}

/* gyr int */
static void icm_pin_int(void *args)
{
    static uint8_t acc_div = 0;
    
    if(rt_icm.parent.rx_indicate)
    {
        rt_icm.parent.rx_indicate(&rt_icm.parent, 1<<RT_SENSOR_POS_GYR_FIFO);
        
        /* down sampling acc to gyr / 5 */
        acc_div++; acc_div %= 5;
        if(!acc_div)
        {
            rt_icm.parent.rx_indicate(&rt_icm.parent, 1<<RT_SENSOR_POS_ACC_FIFO);
        }
        
    }
}


