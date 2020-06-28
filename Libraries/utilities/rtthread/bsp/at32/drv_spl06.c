
#include <rtthread.h>
#include <stdio.h>
#include <rtdevice.h>
#include "common.h"
#include "i2c.h"
#include "gpio.h"

#include "drivers/rt_sensor.h"

#define BSPL06_DEBUG		0
#if ( BSPL06_DEBUG == 1 )
#include <stdio.h>
#define BSPL06_TRACE	printf
#else
#define BSPL06_TRACE(...)
#endif



#define SPL_CHIP_ADDR               (0x77)

#define SPL_PRODUCT_ID_ADDR         0x0D
#define SPL_PRS_CFG_ADDR            0x06
#define SPL_TMP_CFG_ADDR            0x07
#define SPL_CFG_REG_ADDR            0x09
#define SPL_MEAS_CFG_ADDR           0x08
#define SPL_RESET_ADDR              0x0C
#define SPL_TMP_B2_ADDR             0x03
#define SPL_PRS_B2_ADDR             0x00


#define I2C_INSTANCE                (HW_I2C1)



typedef struct
{	
    int16_t c0;
    int16_t c1;
    int32_t c00;
    int32_t c10;
    int16_t c01;
    int16_t c11;
    int16_t c20;
    int16_t c21;
    int16_t c30;
}spl0601_calib_param_t;

static void _spl_dump_cal_param(spl0601_calib_param_t *ctx)
{
    BSPL06_TRACE("c0:%d\r\n", ctx->c0);
    BSPL06_TRACE("c1:%d\r\n", ctx->c1);
    BSPL06_TRACE("c00:%d\r\n", ctx->c00);
    BSPL06_TRACE("c10:%d\r\n", ctx->c10);
    BSPL06_TRACE("c01:%d\r\n", ctx->c01);
    BSPL06_TRACE("c11:%d\r\n", ctx->c11);
    BSPL06_TRACE("c20:%d\r\n", ctx->c20);
    BSPL06_TRACE("c21:%d\r\n", ctx->c21);
    BSPL06_TRACE("c30:%d\r\n", ctx->c30);
}

typedef struct 
{	
    uint8_t chip_addr;
    uint8_t chip_id;
    spl0601_calib_param_t cal_param;
    int32_t kP;    
    int32_t kT;
    int32_t i32rawTemperature;
    int32_t i32rawPressure;
}spl06_t;

static spl06_t spl06;


static uint32_t spl_write_reg(uint8_t reg_addr, uint8_t val)
{
    return I2C_WriteReg(I2C_INSTANCE, spl06.chip_addr, reg_addr, val);
}

static uint32_t spl_read_reg(uint8_t reg_addr, uint8_t *val)
{
    return I2C_ReadReg(I2C_INSTANCE, spl06.chip_addr, reg_addr, val);
}

uint32_t spl_get_prs_tem(float *prs, float *tmp)
{
    uint8_t buf[6];
    int32_t raw_temp, raw_prs;
    
    I2C_BurstRead(I2C_INSTANCE, spl06.chip_addr, SPL_PRS_B2_ADDR, 1, buf, sizeof(buf));
    
    raw_temp = ((int32_t)buf[3]<<16) | ((int32_t)buf[4]<<8) | ((int32_t)buf[5]);
    raw_temp = (raw_temp & 0x800000) ? (0xFF000000 | raw_temp) : raw_temp;
    spl06.i32rawTemperature = raw_temp;
    
    raw_prs = ((int32_t)buf[0]<<16) | ((int32_t)buf[1]<<8) | ((int32_t)buf[2]);
    raw_prs = (raw_prs & 0x800000) ? (0xFF000000 | raw_prs) : raw_prs;
    spl06.i32rawPressure = raw_prs;
    
    float fTsc, fPsc;
    float qua2, qua3;

    /* tmperature */
    fTsc = spl06.i32rawTemperature / (float)spl06.kT;
    *tmp =  spl06.cal_param.c0 * 0.5 + spl06.cal_param.c1 * fTsc;

    /* pressure */
    fTsc = spl06.i32rawTemperature / (float)spl06.kT;
    fPsc = spl06.i32rawPressure / (float)spl06.kP;
    qua2 = spl06.cal_param.c10 + fPsc * (spl06.cal_param.c20 + fPsc* spl06.cal_param.c30);
    qua3 = fTsc * fPsc * (spl06.cal_param.c11 + fPsc * spl06.cal_param.c21);

    *prs = spl06.cal_param.c00 + fPsc * qua2 + fTsc * spl06.cal_param.c01 + qua3;
    return 0;
}

static rt_err_t rt_spl06_init (rt_device_t dev)
{    
    I2C_InitEx(HW_GPIOA, 9, 8, 400*1000);
    return RT_EOK;
}


    
void spl0601_get_calib_param(void)
{
    uint8_t val;

    uint32_t h;
    uint32_t m;
    uint32_t l;
    
    spl_read_reg(0x10, &val);
    h = val;
    
    spl_read_reg(0x11, &val);
    l = val;
    
    spl06.cal_param.c0 = (uint32_t)h<<4 | l>>4;
    spl06.cal_param.c0 = (spl06.cal_param.c0&0x0800)?(0xF000|spl06.cal_param.c0):spl06.cal_param.c0;

    spl_read_reg(0x11, &val);
    h = val;
    
    spl_read_reg(0x12, &val);
    l = val;
    
    spl06.cal_param.c1 = (uint32_t)(h&0x0F)<<8 | l;
    spl06.cal_param.c1 = (spl06.cal_param.c1&0x0800)?(0xF000|spl06.cal_param.c1):spl06.cal_param.c1;
    
    spl_read_reg(0x13, &val);
    h = val;
    
    spl_read_reg(0x14, &val);
    m = val;
    
    spl_read_reg(0x15, &val);
    l = val;
    
    spl06.cal_param.c00 = (int32_t)h<<12 | (int32_t)m<<4 | (int32_t)l>>4;
    spl06.cal_param.c00 = (spl06.cal_param.c00&0x080000)?(0xFFF00000|spl06.cal_param.c00):spl06.cal_param.c00;
    
    spl_read_reg(0x15, &val);
    h = val;
    
    spl_read_reg(0x16, &val);
    m = val;
    
    spl_read_reg(0x17, &val);
    l = val;
    spl06.cal_param.c10 = (int32_t)(h & 0x0F) << 16 | (int32_t)m << 8 | l;
    spl06.cal_param.c10 = (spl06.cal_param.c10&0x080000)?(0xFFF00000|spl06.cal_param.c10):spl06.cal_param.c10;
    
    spl_read_reg(0x18, &val);
    h = val;
    
    spl_read_reg(0x19, &val);
    l = val;
    spl06.cal_param.c01 = (uint32_t)h<<8 | l;
    
    spl_read_reg(0x1A, &val);
    h = val;
    
    spl_read_reg(0x1B, &val);
    l = val;
    spl06.cal_param.c11 = (uint32_t)h<<8 | l;
    
    spl_read_reg(0x1C, &val);
    h = val;
    
    spl_read_reg(0x1D, &val);
    l = val;
    spl06.cal_param.c20 = (uint32_t)h<<8 | l;
    
    spl_read_reg(0x1E, &val);
    h = val;
    
    spl_read_reg(0x1F, &val);
    l = val;
    spl06.cal_param.c21 = (uint32_t)h<<8 | l;
    
    spl_read_reg(0x20, &val);
    h = val;
    
    spl_read_reg(0x21, &val);
    l = val;
    spl06.cal_param.c30 = (uint32_t)h<<8 | l;
}

static rt_err_t rt_spl06_open(rt_device_t dev, rt_uint16_t oflag)
{
    uint8_t val = 0;
    int ret, i;

    spl06.chip_addr = SPL_CHIP_ADDR;
    spl_read_reg(SPL_PRODUCT_ID_ADDR, &val);
    
    if((val & 0xF0) == 0x10)
    {
        spl06.chip_id = val;
        BSPL06_TRACE("SPL06 found addr:0x%X id:0x%X\r\n", spl06.chip_addr, spl06.chip_id);
        
//        spl_write_reg(SPL_RESET_ADDR, 0x89);
//        
//        while(1)
//        {
//            val = 0;
//            spl_read_reg(SPL_MEAS_CFG_ADDR, &val);
//            if(val & ((1<<7) | (1<<6)))
//            {
//                BSPL06_TRACE("coff ready and sensor ready\r\n");
//                break;
//            }
//            rt_thread_delay(rt_tick_from_millisecond(10));
//        }

        for(i=0; i<0x29; i++)
        {
            spl_read_reg(i, &val);
            BSPL06_TRACE("spl reg addr:0x%02X: 0x%02X\r\n", i, val);
        }
        
        spl0601_get_calib_param();
        _spl_dump_cal_param(&spl06.cal_param);
//        case 2:
//            reg |= 1;
//            i32kPkT = 1572864;
//            break;
//        case 4:
//            reg |= 2;
//            i32kPkT = 3670016;
//            break;
//        case 8:
//            reg |= 3;
//            i32kPkT = 7864320;
//            break;
//        case 16:
//            i32kPkT = 253952;
//            reg |= 4;
//            break;
//        case 32:
//            i32kPkT = 516096;
//            reg |= 5;
//            break;
//        case 64:
//            i32kPkT = 1040384;
//            reg |= 6;
//            break;
//        case 128:
//            i32kPkT = 2088960;
//            reg |= 7;
//            break;
        
        spl_write_reg(SPL_PRS_CFG_ADDR, 0x24);
        spl_write_reg(SPL_TMP_CFG_ADDR, 0x24);
        spl06.kP = 253952;
        spl06.kT = 253952;
        spl_write_reg(SPL_CFG_REG_ADDR, (1<<2) | (1<<3));
        
        spl_read_reg(SPL_MEAS_CFG_ADDR, &val);
        val |= 0x07;
        spl_write_reg(SPL_MEAS_CFG_ADDR, val);
        
        ret = RT_EOK;
    }
    else
    {
        ret = RT_EIO;
    }

    return ret;
}

static rt_size_t rt_spl06_read(rt_device_t dev, rt_off_t pos, void* buffer, rt_size_t size)
{
    float t, p;
    spl_get_prs_tem(&p, &t);
    
    switch(pos)
    {
        case RT_SENSOR_POS_PRS:
            
            rt_memcpy(buffer, (uint8_t*)&p, sizeof(p));
            break;
        case RT_SENSOR_POS_PRS_TEMPERATURE:
            rt_memcpy(buffer, (uint8_t*)&t, sizeof(t));
            break;
    }
    return size;
}

static rt_size_t rt_spl06_write(rt_device_t dev, rt_off_t pos, const void* buffer, rt_size_t size)
{   
    return size;
}

static rt_err_t  rt_spl06_control(rt_device_t dev, int cmd, void *args)
{
    return RT_EOK;
}

int rt_hw_spl06_init(const char *name)
{
    struct rt_device *dev;
    
    dev                     = rt_malloc(sizeof(struct rt_device));
	dev->type               = RT_Device_Class_Miscellaneous;
	dev->rx_indicate        = RT_NULL;
	dev->tx_complete        = RT_NULL;
	dev->init               = rt_spl06_init;
	dev->open               = rt_spl06_open;
	dev->close              = RT_NULL;
	dev->read               = rt_spl06_read;
	dev->write              = rt_spl06_write;
	dev->control            = rt_spl06_control;
	dev->user_data          = RT_NULL;
 
    rt_device_register(dev, name, RT_DEVICE_FLAG_RDWR);
    return RT_EOK;
}


