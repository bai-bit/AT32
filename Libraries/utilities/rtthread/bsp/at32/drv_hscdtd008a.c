
#include <rtthread.h>
#include <stdio.h>
#include <rtdevice.h>
#include "common.h"
#include "i2c.h"
#include "gpio.h"

#include "drivers/rt_sensor.h"


#define HSCD_DEBUG		0
#if ( HSCD_DEBUG == 1 )
#include <stdio.h>
#define HSCD_TRACE	printf
#else
#define HSCD_TRACE(...)
#endif


#define HSCD_STB         0x0C
#define HSCD_XOUT_H      0x11
#define HSCD_XOUT_L      0x10
#define HSCD_YOUT_H      0x13
#define HSCD_YOUT_L      0x12
#define HSCD_ZOUT_H      0x15
#define HSCD_ZOUT_L      0x14

#define HSCD_STATUS      0x18
#define HSCD_CTRL1       0x1b
#define HSCD_CTRL2       0x1c
#define HSCD_CTRL3       0x1d
#define HSCD_CTRL4       0x1e

#define HSCD_TCS_TIME    10000    /* Measure temp. of every 10 sec */
/* hscdtd008a chip id */
#define DEVICE_ID       0x49
/* hscd magnetic sensor chip identification register */
#define WHO_AM_I        0x0F
#define RETRY_COUNT	10



typedef struct 
{	
    uint8_t chip_addr;
    uint8_t chip_id;
}hscd_t;

static hscd_t hscd;

#define I2C_INSTANCE                (HW_I2C1)


static uint32_t hscd_write_reg(uint8_t reg_addr, uint8_t val)
{
    return I2C_WriteReg(I2C_INSTANCE, hscd.chip_addr, reg_addr, val);
}

static uint32_t hscd_read_reg(uint8_t reg_addr, uint8_t *val)
{
    return I2C_ReadReg(I2C_INSTANCE, hscd.chip_addr, reg_addr, val);
}


static rt_err_t rt_hscd_init (rt_device_t dev)
{    
    I2C_InitEx(HW_GPIOA, 9, 8, 400*1000);
    return RT_EOK;
}

static rt_err_t rt_hscd_open(rt_device_t dev, rt_uint16_t oflag)
{
    int i;
    uint8_t val = 0;
    rt_err_t ret;
    
    I2C_Scan(I2C_INSTANCE);
    hscd.chip_addr = 0x0C;
    hscd_read_reg(WHO_AM_I, &val);
    if(val == 0x49)
    {
        hscd.chip_id = 0x49;
        HSCD_TRACE("HSCDTD008A found! ADDR:0x%X ID:0x%X\r\n", hscd.chip_addr, hscd.chip_id);
        
        HSCD_TRACE("dump register:\r\n");
        for(i=0; i<0x32; i++)
        {
            hscd_read_reg(i, &val);
            HSCD_TRACE("[0x%X]:0x%X\r\n", i, val);
        }
        
        hscd_write_reg(HSCD_CTRL1, 0x98); /* 100Hz output */
        
        ret = RT_EOK;
    }
    else
    {
        HSCD_TRACE("HSCDTD008A not found!: ADDR:0x%X ID:0x%X\r\n", hscd.chip_addr, hscd.chip_id);
        ret = RT_EIO;
    }
    
    return ret;
}


static rt_size_t rt_hscd_read(rt_device_t dev, rt_off_t pos, void* buffer, rt_size_t size)
{
    int16_t buf[3];
    float *p = (float*)buffer;
    
    I2C_BurstRead(I2C_INSTANCE, hscd.chip_addr, HSCD_XOUT_L, 1, (uint8_t*)buf, sizeof(buf));
    /* 1 uT = 10 mGauss,  SENSOR: 0.15uT/LSB */
    
    p[0] = buf[0] * 1.5;
    p[1] = buf[1] * 1.5;
    p[2] = buf[2] * 1.5;
    return size;
}

static rt_size_t rt_hscd_write(rt_device_t dev, rt_off_t pos, const void* buffer, rt_size_t size)
{
    return size;
}

static rt_err_t  rt_hscd_control(rt_device_t dev, int cmd, void *args)
{
    return RT_EOK;
}

int rt_hw_hscdtd008a_init(const char *name)
{
    struct rt_device *dev;
    
    dev                     = rt_malloc(sizeof(struct rt_device));
	dev->type               = RT_Device_Class_Miscellaneous;
	dev->rx_indicate        = RT_NULL;
	dev->tx_complete        = RT_NULL;
	dev->init               = rt_hscd_init;
	dev->open               = rt_hscd_open;
	dev->close              = RT_NULL;
	dev->read               = rt_hscd_read;
	dev->write              = rt_hscd_write;
	dev->control            = rt_hscd_control;
	dev->user_data          = RT_NULL;
 
    rt_device_register(dev, name, RT_DEVICE_FLAG_RDWR);
    return RT_EOK;
}


