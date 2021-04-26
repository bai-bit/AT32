
#include <rtthread.h>
#include <rtdevice.h>


#define QMC5883_CHIP_ADDR       (0x0D)

#define QMC_DEBUG		0
#if ( QMC_DEBUG == 1 )
#include <stdio.h>
#define QMC_TRACE	printf
#else
#define QMC_TRACE(...)
#endif

typedef struct 
{
    struct rt_device            parent;
    struct rt_i2c_bus_device    *bus;
    uint8_t                     rst_pin;
    uint8_t                     chip_addr;
}qmc_device_t;


static uint8_t _qmc_read_reg(qmc_device_t *dev, uint8_t reg_addr)
{
    uint8_t val;
    
    rt_i2c_master_send(dev->bus, dev->chip_addr, 0, &reg_addr, 1);
    rt_i2c_master_recv(dev->bus, dev->chip_addr, 0, &val, 1);
    return val;
}

static uint32_t _qmc_write_reg(qmc_device_t *dev, uint8_t reg_addr, uint8_t val)
{
    uint8_t buf[2];
    
    buf[0] = reg_addr;
    buf[1] = val;
    return rt_i2c_master_send(dev->bus, dev->chip_addr, 0, buf, sizeof(buf));
}



//int qmc5883_set_odr(qmc_odr_t odr)
//{
//    uint8_t data;
//    read_reg(0x09, &data);
//    data &= ~0x0C;
//    switch(odr)
//    {
//        case kQMC_10Hz:
//            data |= (0<<2);
//            break;
//        case kQMC_50Hz:
//            data |= (1<<2);
//            break;
//        case kQMC_100Hz:
//            data |= (2<<2);
//            break;
//        case kQMC_200Hz:
//            data |= (3<<2);
//            break;
//        default:
//            break;
//    }
//    write_reg(0x09, data);
//    return CH_OK;
//}

///*
//    1 T = 10,000 Gauss 
//    1uT = 0.01 Gauss
//    1uT = 10mG
//    0.1uT = 1mG
//    100uT = 1Gauss
//*/

//int qmc5883_set_range(qmc_range_t range)
//{
//    uint8_t data;
//    read_reg(0x09, &data);
//    data &= ~0x30;
//    switch(range)
//    {
//        case kQMC_2G:
//            data |= (0<<4);
//            break;
//        case kQMC_8G:
//            data |= (1<<4);
//            break;
//        default:
//            break;
//    }
//    write_reg(0x09, data);
//    return CH_OK;
//}


///*
//Over sample Rate (OSR) registers are used to control bandwidth of an internal digital filter. 
//Larger OSR value leads to smaller filter bandwidth, less in-band noise and higher power consumption.
//It could be used to reach a good balance between noise and power. Four over sample ratio can be selected, 64, 128, 256 or 512.
//*/

//int qmc5883_set_osr(qmc_osr_t osr)
//{
//    uint8_t data;
//    read_reg(0x09, &data);
//    data &= ~0xC0;
//    switch(osr)
//    {
//        case kQMC_OSR_512:
//            data |= (0<<6);
//            break;
//        case kQMC_OSR_256:
//            data |= (1<<6);
//            break;
//        case kQMC_OSR_128:
//            data |= (2<<6);
//            break;
//        case kQMC_OSR_64:
//            data |= (3<<6);
//            break;
//        default:
//            break;
//    }
//    write_reg(0x09, data);
//    return CH_OK;
//}

static rt_err_t rt_qmc_init(rt_device_t dev)
{    
    return RT_EOK;
}

static rt_err_t rt_qmc_open(rt_device_t dev, rt_uint16_t oflag)
{
    int i;
    uint8_t val;
    rt_err_t ret;
    
    qmc_device_t *qmc_dev = (qmc_device_t *)dev;
    
    /* retry 3 tiems */
    for(i=0; i<3; i++)
    {
        _qmc_write_reg(qmc_dev, 0x09, 0x09); /* 100Hz output */
        val = _qmc_read_reg(qmc_dev, 0x09);
        if(val == 0x09)
        {
            QMC_TRACE("qmc5883 ok\r\n");
            _qmc_write_reg(qmc_dev, 0x0B, 0x01); /* SET/RESET Period Register */
            _qmc_write_reg(qmc_dev, 0x20, 0x40);
            _qmc_write_reg(qmc_dev, 0x21, 0x01);
            _qmc_write_reg(qmc_dev, 0x09, 0x09);
            _qmc_write_reg(qmc_dev, 0x0A, 0x00);
            
            ret = RT_EOK;
            return ret;
        }
        else
        {
            QMC_TRACE("qmc5883 fail!\r\n");
            ret = RT_EIO;
        }
    }

    return ret;
}

static void qmc5883_raw_to_uT(int16_t *raw, float *phy_raw)
{
    float ratio = 1.0F/120;
    
    phy_raw[0] = ((float)raw[0])*(ratio);
    phy_raw[1] = ((float)raw[1])*(ratio);
    phy_raw[2] = ((float)raw[2])*(ratio);
}

static rt_size_t rt_qmc_read(rt_device_t dev, rt_off_t pos, void* buffer, rt_size_t size)
{
    qmc_device_t *qmc_dev = (qmc_device_t *)dev;
    
    uint8_t rx_buf[7];
    uint8_t reg_addr = 0x00;
    
    rt_i2c_master_send(qmc_dev->bus, qmc_dev->chip_addr, 0, &reg_addr, 1);
    rt_i2c_master_recv(qmc_dev->bus, qmc_dev->chip_addr, 0, rx_buf, 7);
    qmc5883_raw_to_uT((int16_t*)rx_buf, buffer);
    
    return size;
}


int rt_hw_qmc5883_init(const char *bus_name, const char *name, uint8_t rst_pin)
{
    static qmc_device_t qmc_drv;
    struct rt_i2c_bus_device *bus;

    bus = rt_i2c_bus_device_find(bus_name);
    if (bus == RT_NULL)
    {
        return RT_ENOSYS;
    }
    
    qmc_drv.parent.type               = RT_Device_Class_Miscellaneous;
	qmc_drv.parent.rx_indicate        = RT_NULL;
	qmc_drv.parent.tx_complete        = RT_NULL;
	qmc_drv.parent.init               = rt_qmc_init;
	qmc_drv.parent.open               = rt_qmc_open;
	qmc_drv.parent.close              = RT_NULL;
	qmc_drv.parent.read               = rt_qmc_read;
	qmc_drv.parent.write              = RT_NULL;
	qmc_drv.parent.user_data          = RT_NULL;
    
    qmc_drv.bus = bus;
    qmc_drv.rst_pin = rst_pin;
    qmc_drv.chip_addr = QMC5883_CHIP_ADDR;
 
    rt_device_register(&qmc_drv.parent, name, RT_DEVICE_FLAG_RDWR);
    return RT_EOK;
}

