#include <rtthread.h>
#include <stdio.h>
#include <rtdevice.h>
#include "common.h"
#include "bmi055.h"
#include "spi.h"
#include "gpio.h"

#include "drivers/rt_sensor.h"

static const uint32_t acc_rg_tbl[] = {2, 4, 8, 16};
static const uint32_t acc_bw_tbl[] = {8, 16, 32, 62, 125, 250, 500};
static const uint32_t gyr_rg_tbl[] = {250, 500, 1000, 2000};
static const uint32_t gyr_bw_tbl[] = {12, 23, 32, 47, 64, 116, 230, 523};


static void bmi055_gyr_pin_int(void *args);
static void bmi055_acc_pin_int(void *args);

typedef struct
{
    struct rt_device parent;
    uint32_t acc_rg;
    uint32_t acc_bw;
    uint32_t gyr_rg;
    uint32_t gyr_bw;
    uint8_t this_corrd;
    uint32_t cs;
    uint32_t acc_int_pin;
    uint32_t gyr_int_pin;
}rt_bmi_t;

static rt_bmi_t rt_bmi;

static void select_cs(uint32_t chip)
{
    rt_bmi.cs = chip;
}

static void set_cs(uint32_t chip, uint32_t state)
{
    if(chip == 0)
    {
        GPIO_PinWrite(HW_GPIOA, 16, state); /* GY0 */
    }
    
    if(chip == 1)
    {
        GPIO_PinWrite(HW_GPIOA, 14, state); /* ACC */
    }
}

static uint32_t write_data(uint8_t *buf, uint32_t len, uint8_t cs)
{
    set_cs(rt_bmi.cs, 0);
    SPI_ReadWriteFIFO(HW_SPI0, buf, NULL, len);
    set_cs(rt_bmi.cs, cs);
    return CH_OK;
}

static uint32_t read_data(uint8_t *buf, uint32_t len, uint8_t cs)
{
    set_cs(rt_bmi.cs, 0);
    SPI_ReadWriteFIFO(HW_SPI0, NULL, buf, len);
    set_cs(rt_bmi.cs, cs);
    return CH_OK;
}

static struct bmi055_init_t bmi_ops = 
{
    select_cs,
    write_data,
    read_data,
    DelayMs,
};


static rt_err_t rt_bmi055_init (rt_device_t dev)
{    
    SPI_Init(HW_SPI0, 8*1000*1000);
    
    #if (APP_PN == 221)
    GPIO_Init(HW_GPIOA, 14, kGPIO_OPPH); /* ACC CS 0xFA */
    GPIO_Init(HW_GPIOA, 16, kGPIO_OPPH); /* GYR CS 0x0F */
    
    
    NRF_SPIM0->PSEL.MISO = 15;
    NRF_SPIM0->PSEL.MOSI = 13;
    NRF_SPIM0->PSEL.SCK = 12;
    
    #elif(APP_PN == 220)
    GPIO_Init(HW_GPIOA, 14, kGPIO_OPPH); /* ACC CS 0xFA */
    GPIO_Init(HW_GPIOA, 16, kGPIO_OPPH); /* GYR CS 0x0F */
    
    NRF_SPIM0->PSEL.MISO = 15;
    NRF_SPIM0->PSEL.MOSI = 12;
    NRF_SPIM0->PSEL.SCK = 10;
    #endif
    
    
    return RT_EOK;
}

static rt_err_t rt_bmi055_open(rt_device_t dev, rt_uint16_t oflag)
{
    rt_err_t ret;
    
    ret = bmi055_init(&bmi_ops);
    if(ret == CH_OK)
    {
        ret = RT_EOK;
    }
    else
    {
        ret = RT_EIO;
    }
    
    /* enable raw sensor acc interrupt */
    rt_pin_mode(rt_bmi.acc_int_pin, PIN_MODE_INPUT_PULLUP);
    rt_pin_attach_irq(rt_bmi.acc_int_pin, PIN_IRQ_MODE_RISING, bmi055_acc_pin_int, RT_NULL);
    rt_pin_irq_enable(rt_bmi.acc_int_pin, PIN_IRQ_ENABLE);

    /* enable raw sensor gyr interrupt */
    rt_pin_mode(rt_bmi.gyr_int_pin, PIN_MODE_INPUT_PULLUP);
    rt_pin_attach_irq(rt_bmi.gyr_int_pin, PIN_IRQ_MODE_RISING, bmi055_gyr_pin_int, RT_NULL);
    rt_pin_irq_enable(rt_bmi.gyr_int_pin, PIN_IRQ_ENABLE);
    
    return ret;
}


static rt_size_t rt_bmi055_read(rt_device_t dev, rt_off_t pos, void* buffer, rt_size_t size)
{
    switch(pos)
    {
        case RT_SENSOR_POS_ACC:
            bmi055_read_acc(buffer);
            break;
        case RT_SENSOR_POS_GYR:
            bmi055_read_gyr(buffer);
            break;
        case RT_SENSOR_POS_ACC_FIFO:
            bmi055_read_acc_fifo(buffer, size);
            break;
        case RT_SENSOR_POS_GYR_FIFO:
            bmi055_read_gyr_fifo(buffer, size);
            break;
        case RT_SENSOR_POS_GYR_TEMPERATURE:
            bmi055_read_acc_temperature(buffer);
            break;
        case RT_SENSOR_POS_ACC_BW:
            rt_memcpy(buffer, &acc_bw_tbl[rt_bmi.acc_bw], 4);
            break;
        case RT_SENSOR_POS_ACC_RG:
            rt_memcpy(buffer, &acc_rg_tbl[rt_bmi.acc_rg], 4);
            break;
        case RT_SENSOR_POS_GYR_RG:
            rt_memcpy(buffer, &gyr_rg_tbl[rt_bmi.gyr_rg], 4);
            break;
        case RT_SENSOR_POS_GYR_BW:
            rt_memcpy(buffer, &gyr_bw_tbl[rt_bmi.gyr_bw], 4);
            break;
    }
    return size;
}

static rt_size_t rt_bmi055_write(rt_device_t dev, rt_off_t pos, const void* buffer, rt_size_t size)
{   
    uint8_t *val = (uint8_t *)buffer;
    
    switch(pos)
    {
        case RT_SENSOR_POS_ACC_RG:
            rt_bmi.acc_rg = *val;
            switch(*val)
            {
                case 0:
                    bmi_set_acc_range(kBMI_ACC_2G);
                    break;
                case 1:
                    bmi_set_acc_range(kBMI_ACC_4G);
                    break;
                case 2:
                    bmi_set_acc_range(kBMI_ACC_8G);
                    break;
                case 3:
                    bmi_set_acc_range(kBMI_ACC_16G);
                    break;
            }
            break;
        case RT_SENSOR_POS_GYR_RG:
            rt_bmi.gyr_rg = *val;
            switch(*val)
            {
                case 0:
                    bmi_set_gyr_range(kBMI_250DPS);
                    break;
                case 1:
                    bmi_set_gyr_range(kBMI_500DPS);
                    break;
                case 2:
                    bmi_set_gyr_range(kBMI_1000DPS);
                    break;
                case 3:
                    bmi_set_gyr_range(kBMI_2000DPS);
                    break;
            }
            break;
        case RT_SENSOR_POS_ACC_BW:
            rt_bmi.acc_bw = *val;
            switch(*val)
            {
                case 0:
                    bmi_set_acc_bw(kBMI_ACC_8Hz);
                    break;
                case 1:
                    bmi_set_acc_bw(kBMI_ACC_16Hz);
                    break;
                case 2:
                    bmi_set_acc_bw(kBMI_ACC_32Hz);
                    break;
                case 3:
                    bmi_set_acc_bw(kBMI_ACC_62Hz);
                    break;
                case 4:
                    bmi_set_acc_bw(kBMI_ACC_125Hz);
                    break;
                case 5:
                    bmi_set_acc_bw(kBMI_ACC_250Hz);
                    break;
                case 6:
                    bmi_set_acc_bw(kBMI_ACC_500Hz);
                    break;
                default:
                    break;
            }
            break;
        case RT_SENSOR_POS_GYR_BW:
            rt_bmi.gyr_bw = *val;
            switch(*val)
            {
                case 0:
                    bmi_set_gyr_bw(kBMI_GYR_12Hz);
                    break;
                case 1:
                    bmi_set_gyr_bw(kBMI_GYR_23Hz);
                    break;
                case 2:
                    bmi_set_gyr_bw(kBMI_GYR_32Hz);
                    break;
                case 3:
                    bmi_set_gyr_bw(kBMI_GYR_47Hz);
                    break;
                case 4:
                    bmi_set_gyr_bw(kBMI_GYR_64Hz);
                    break;
                case 5:
                    bmi_set_gyr_bw(kBMI_GYR_116Hz);
                    break;
                case 6:
                    bmi_set_gyr_bw(kBMI_GYR_230Hz);
                    break;
                case 7:
                    bmi_set_gyr_bw(kBMI_GYR_523Hz);
                    break;
                default:

                    break;
            }
        case RT_SENSOR_POS_ACC_CLEAR_FIFO:
            bmi055_clear_acc_fifo();
            break;
        case RT_SENSOR_POS_GYR_CLEAR_FIFO:
            bmi055_clear_gyr_fifo();
            break;
        case RT_SENSOR_POS_ACC_WM_FIFO_INT:
            bmi055_acc_enable_fifo_wm_int();
            break;
        case RT_SENSOR_POS_GYR_WM_FIFO_INT:
            bmi055_gyr_enable_fifo_wm_int();
            break;
    }
    return size;
}

static rt_err_t  rt_bmi055_control(rt_device_t dev, int cmd, void *args)
{
    
    return RT_EOK;
}

int rt_hw_bmi055_init(const char *name, uint32_t acc_int_pin, uint32_t gyr_int_pin)
{
    struct rt_device *dev;
    
    dev                     = &rt_bmi.parent;
	dev->type               = RT_Device_Class_Miscellaneous;
	dev->rx_indicate        = RT_NULL;
	dev->tx_complete        = RT_NULL;
	dev->init               = rt_bmi055_init;
	dev->open               = rt_bmi055_open;
	dev->close              = RT_NULL;
	dev->read               = rt_bmi055_read;
	dev->write              = rt_bmi055_write;
	dev->control            = rt_bmi055_control;
	dev->user_data          = &rt_bmi;
 
    rt_bmi.acc_int_pin = acc_int_pin;
    rt_bmi.gyr_int_pin = gyr_int_pin;
    
    rt_device_register(dev, name, RT_DEVICE_FLAG_RDWR);
    return RT_EOK;
}


/* gyr int */
static void bmi055_gyr_pin_int(void *args)
{
    if(rt_bmi.parent.rx_indicate)
    {
        rt_bmi.parent.rx_indicate(&rt_bmi.parent, 1<<RT_SENSOR_POS_GYR_FIFO);
    }
}

/* acc int */
static void bmi055_acc_pin_int(void *args)
{
    if(rt_bmi.parent.rx_indicate)
    {
        rt_bmi.parent.rx_indicate(&rt_bmi.parent, 1<<RT_SENSOR_POS_ACC_FIFO);
    }
}










