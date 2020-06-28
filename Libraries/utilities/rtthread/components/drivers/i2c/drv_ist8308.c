
#include <rtthread.h>
#include <stdio.h>
#include <rtdevice.h>

#define WHO_AM_I_8308 (0x08)
#define WHO_AM_I_8310 (0x10)

#define IST8308_DEBUG		0
#if ( IST8308_DEBUG == 1 )
#include <stdio.h>
#define IST8308_TRACE	printf
#else
#define IST8308_TRACE(...)
#endif

typedef struct 
{
    struct rt_device            parent;
    struct rt_i2c_bus_device    *bus;
    uint8_t                     rst_pin;
    uint8_t                     chip_addr;
}ist_device_t;


static uint8_t _ist_read_reg(ist_device_t *dev, uint8_t reg_addr)
{
    uint8_t val;
    
    rt_i2c_master_send(dev->bus, dev->chip_addr, 0, &reg_addr, 1);
    rt_i2c_master_recv(dev->bus, dev->chip_addr, 0, &val, 1);
    return val;
}

static uint32_t _ist_write_reg(ist_device_t *dev, uint8_t reg_addr, uint8_t val)
{
    uint8_t buf[2];
    
    buf[0] = reg_addr;
    buf[1] = val;
    
    return rt_i2c_master_send(dev->bus, dev->chip_addr, 0, buf, 2);
}


static rt_err_t rt_ist_init (rt_device_t dev)
{    
    return RT_EOK;
}

static rt_err_t rt_ist_open(rt_device_t dev, rt_uint16_t oflag)
{
    uint8_t val;
    rt_err_t ret;

    ist_device_t *ist_dev = (ist_device_t *)dev;

    rt_pin_mode(ist_dev->rst_pin, PIN_MODE_OUTPUT); /* IST8308 RST */
    
    /* issue hardware rst */
    rt_pin_write(ist_dev->rst_pin, 0);
    rt_thread_delay(rt_tick_from_millisecond(2));
    rt_pin_write(ist_dev->rst_pin, 1);
    rt_thread_delay(rt_tick_from_millisecond(10));
    
    val = _ist_read_reg(ist_dev, 0x00);
    if(val == WHO_AM_I_8308)
    {
        IST8308_TRACE("ist8308 ok, addr:0x%X ID:0x%X\r\n", ist_dev->chip_addr, val);
        _ist_write_reg(ist_dev, 0x30, 0<<5); /* low level of LPF */
        _ist_write_reg(ist_dev, 0x31, 0x0A); /* 100 Hz */
        _ist_write_reg(ist_dev, 0x41, (4<<0) | (4<<3));  /* LPF: 100 Hz */
        _ist_write_reg(ist_dev, 0x34, 1); /* 2 Gauss */
        ret = RT_EOK;
    }
    else
    {
        ret = RT_EIO;
        IST8308_TRACE("IST8308 init failed\r\n");
    }

    return ret;
}


/*
    1 Tesla = 10,000 Gauss  1uT = 0.01 Gauss 1uT = 10mG 0.1uT = 1mG 100uT = 1Gauss
*/   
void ist8308_raw_to_uT(int16_t *raw, float *phy_raw)
{
    float ratio = 1 / 13.2F;
    phy_raw[0] = ((float)raw[0])*(ratio);
    phy_raw[1] = ((float)raw[1])*(ratio);
    phy_raw[2] = ((float)raw[2])*(ratio);
}



static rt_size_t rt_ist_read(rt_device_t dev, rt_off_t pos, void* buffer, rt_size_t size)
{
    ist_device_t *ist_dev = (ist_device_t *)dev;
    
    uint8_t rx_buf[7];
    uint8_t reg_addr = 0x10;
    
    rt_i2c_master_send(ist_dev->bus, ist_dev->chip_addr, 0, &reg_addr, 1);
    rt_i2c_master_recv(ist_dev->bus, ist_dev->chip_addr, 0, rx_buf, 7);
    ist8308_raw_to_uT((int16_t*)&rx_buf[1], buffer);
    
    /* swap X Y */
    float *fbuf = (float *)buffer;
    float tmp = fbuf[0];
    fbuf[0] = fbuf[1];
    fbuf[1] = tmp;
    return size;
}

static rt_size_t rt_ist_write(rt_device_t dev, rt_off_t pos, const void* buffer, rt_size_t size)
{   
    return size;
}


int rt_hw_ist8308_init(const char *bus_name, const char *name, uint8_t rst_pin)
{
    static ist_device_t ist_drv;
    
    struct rt_i2c_bus_device *bus;

    bus = rt_i2c_bus_device_find(bus_name);
    if (bus == RT_NULL)
    {
        return RT_ENOSYS;
    }
    
    ist_drv.parent.type               = RT_Device_Class_Miscellaneous;
	ist_drv.parent.rx_indicate        = RT_NULL;
	ist_drv.parent.tx_complete        = RT_NULL;
	ist_drv.parent.init               = rt_ist_init;
	ist_drv.parent.open               = rt_ist_open;
	ist_drv.parent.close              = RT_NULL;
	ist_drv.parent.read               = rt_ist_read;
	ist_drv.parent.write              = rt_ist_write;
	ist_drv.parent.user_data          = RT_NULL;
    
    ist_drv.bus = bus;
    ist_drv.rst_pin = rst_pin;
    ist_drv.chip_addr = 0x0C;
 
    rt_device_register(&ist_drv.parent, name, RT_DEVICE_FLAG_RDWR);
    return RT_EOK;
}


