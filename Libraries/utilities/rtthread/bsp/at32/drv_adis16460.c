#include <rtthread.h>
#include <stdio.h>
#include <rtdevice.h>
#include "common.h"
#include "spi.h"
#include "dma.h"
#include "gpio.h"

#include "drivers/rt_sensor.h"


#define ADIS_DEBUG		1
#if ( ADIS_DEBUG == 1 )
#include <stdio.h>
#define ADIS_TRACE	printf
#else
#define ADIS_TRACE(...)
#endif

#define FLASH_CNT   0x00  //Flash memory write count
#define DIAG_STAT   0x02  //Diagnostic and operational status
#define X_GYRO_LOW  0x04  //X-axis gyroscope output, lower word
#define X_GYRO_OUT  0x06  //X-axis gyroscope output, upper word
#define Y_GYRO_LOW  0x08  //Y-axis gyroscope output, lower word
#define Y_GYRO_OUT  0x0A  //Y-axis gyroscope output, upper word
#define Z_GYRO_LOW  0x0C  //Z-axis gyroscope output, lower word
#define Z_GYRO_OUT  0x0E  //Z-axis gyroscope output, upper word
#define X_ACCL_LOW  0x10  //X-axis accelerometer output, lower word
#define X_ACCL_OUT  0x12  //X-axis accelerometer output, upper word
#define Y_ACCL_LOW  0x14  //Y-axis accelerometer output, lower word
#define Y_ACCL_OUT  0x16  //Y-axis accelerometer output, upper word
#define Z_ACCL_LOW  0x18  //Z-axis accelerometer output, lower word
#define Z_ACCL_OUT  0x1A  //Z-axis accelerometer output, upper word
#define SMPL_CNTR   0x1C  //Sample Counter, MSC_CTRL[3:2]=11
#define TEMP_OUT    0x1E  //Temperature output (internal, not calibrated)
#define X_DELT_ANG  0x24  //X-axis delta angle output
#define Y_DELT_ANG  0x26  //Y-axis delta angle output
#define Z_DELT_ANG  0x28  //Z-axis delta angle output
#define X_DELT_VEL  0x2A  //X-axis delta velocity output
#define Y_DELT_VEL  0x2C  //Y-axis delta velocity output
#define Z_DELT_VEL  0x2E  //Z-axis delta velocity output
#define MSC_CTRL    0x32  //Miscellaneous control
#define SYNC_SCAL   0x34  //Sync input scale control
#define DEC_RATE    0x36  //Decimation rate control
#define FLTR_CTRL   0x38  //Filter control, auto-null record time
#define GLOB_CMD    0x3E  //Global commands
#define XGYRO_OFF   0x40  //X-axis gyroscope bias offset error
#define YGYRO_OFF   0x42  //Y-axis gyroscope bias offset error
#define ZGYRO_OFF   0x44  //Z-axis gyroscope bias offset factor
#define XACCL_OFF   0x46  //X-axis acceleration bias offset factor
#define YACCL_OFF   0x48  //Y-axis acceleration bias offset factor
#define ZACCL_OFF   0x4A  //Z-axis acceleration bias offset factor
#define LOT_ID1     0x52  //Lot identification number
#define LOT_ID2     0x54  //Lot identification number
#define PROD_ID     0x56  //Product identifier
#define SERIAL_NUM  0x58  //Lot-specific serial number
#define CAL_SGNTR   0x60  //Calibration memory signature value
#define CAL_CRC     0x62  //Calibration memory CRC values
#define CODE_SGNTR  0x64  //Code memory signature value
#define CODE_CRC    0x66  //Code memory CRC values


static void adis_pin_int(void *args);

typedef struct
{
    struct rt_device        parent;
    uint32_t                int_pin;
    uint8_t                 tx_buf[20];
    uint8_t                 rx_buf[20];
    float                   gyr_ratio;
    float                   acc_ratio;
    rt_sem_t                sem;
    rt_mutex_t              mtx;
}rt_adis_t;


static rt_adis_t rt_adis;


static void adis_set_cs(uint32_t state)
{
    GPIO_PinWrite(HW_GPIOB, 12, state);
}

static uint32_t write_data(uint8_t *buf, uint32_t len, uint8_t cs)
{
	int i;
    adis_set_cs(0);

	for(i=0; i<len; i++)
	{
		SPI_ReadWrite(HW_SPI1, buf[i]);
	}
    
    adis_set_cs(cs);
    return CH_OK;
}

static uint32_t read_data(uint8_t *buf, uint32_t len, uint8_t cs)
{
    int i;
    adis_set_cs(0);

	for(i=0; i<len; i++)
	{
		buf[i] = SPI_ReadWrite(HW_SPI1, 0xFF);
	}

    adis_set_cs(cs);
    return CH_OK;
}


static void adis_burst_read(uint8_t addr, uint8_t *rx_buf, uint32_t len)
{
    rt_adis.tx_buf[0] = addr;
    rt_adis.tx_buf[1] = 0x00;
    
    adis_set_cs(0);

    SPI_ReadWrite(HW_SPI1, rt_adis.tx_buf[0]);
    SPI_ReadWrite(HW_SPI1, rt_adis.tx_buf[1]);
    read_data(rt_adis.rx_buf, len, 1);
}


//static void adis_async_burst_read(uint8_t addr, uint8_t *rx_buf, uint32_t len)
//{
//    rt_adis.tx_buf[0] = addr;
//    rt_adis.tx_buf[1] = 0x00;
//    
//    adis_set_cs(0);
//    
//    SPI_ReadWrite(HW_SPI1, rt_adis.tx_buf[0]);
//    SPI_ReadWrite(HW_SPI1, rt_adis.tx_buf[1]);
//    
//    SPI_StartDMATxRx(HW_SPI1, rt_adis.tx_buf, rx_buf, len);
//}


static uint16_t adis16460_read_reg(uint8_t addr)
{
    uint8_t dat[2];
    
    dat[0] = addr;
    dat[1] = 0x00;
    
    write_data(dat, sizeof(dat), 1);
    rt_thread_delay(rt_tick_from_millisecond(3));
    read_data(dat, sizeof(dat), 1);
    return (dat[0]<<8) + dat[1];
}

static void adis16460_write_register(uint8_t regAddr, int16_t regData)
{
    /* Write register address and data */
    uint16_t addr = (((regAddr & 0x7F) | 0x80) << 8); // Toggle sign bit, and check that the address is 8 bits
    uint16_t lowWord = (addr | (regData & 0xFF)); // OR Register address (A) with data(D) (AADD)
    uint16_t highWord = ((addr | 0x100) | ((regData >> 8) & 0xFF)); // OR Register address with data and increment address

    /* Split words into chars */
    uint8_t highBytehighWord = (highWord >> 8);
    uint8_t lowBytehighWord = (highWord & 0xFF);
    uint8_t highBytelowWord = (lowWord >> 8);
    uint8_t lowBytelowWord = (lowWord & 0xFF);

    /* Write highWord to SPI bus */
    write_data((uint8_t*)&highBytelowWord, 1, 0);
    write_data((uint8_t*)&lowBytelowWord, 1, 1);

    rt_thread_delay(rt_tick_from_millisecond(10));

    /* Write lowWord to SPI bus */
    write_data((uint8_t*)&highBytehighWord, 1, 0);
    write_data((uint8_t*)&lowBytehighWord, 1, 1);
}

static rt_err_t rt_adis16460_init (rt_device_t dev)
{    
    /* RST */
    GPIO_Init(HW_GPIOA, 1,  kGPIO_OPPL);
    rt_thread_delay(rt_tick_from_millisecond(1));
    GPIO_Init(HW_GPIOA, 1,  kGPIO_OPPH);
    rt_thread_delay(rt_tick_from_millisecond(300));
    
    GPIO_Init(HW_GPIOB, 12, kGPIO_OPPH); /* CS */
	
    SPI_Init(HW_SPI1, 1*1000*1000);
    SPI_SetMode(HW_SPI1, 1, 1);
    
    DMA_Init(HW_DMA1);
    SPI_DMAInit(HW_SPI1);
    DMA_SetChlIntMode(HW_DMA1_SPI_RX, 1);
    
    SetPinMux(HW_GPIOA,  5, 2, 2);   /* CLK */
    SetPinMux(HW_GPIOA,  6, 1, 0);   /* MISO */
    SetPinMux(HW_GPIOA,  7, 2, 2);   /* MOSI */
	
    
    return RT_EOK;
}


static int16_t _checksum(uint8_t * buf)
{
    int i;
	int16_t s = 0;
	for(i=0; i<18; i++)
	{
        s += buf[i];
	}
	return s;
}


void thread_adis_entry(void* parameter)
{
    while(1)
    {
        rt_sem_take(rt_adis.sem, rt_tick_from_millisecond(10));
        
        rt_mutex_take(rt_adis.mtx, RT_WAITING_FOREVER);
        adis_burst_read(0x3E, rt_adis.rx_buf, sizeof(rt_adis.rx_buf));
        rt_mutex_release(rt_adis.mtx);
    }
}


static rt_err_t rt_adis16460_open(rt_device_t dev, rt_uint16_t oflag)
{
    int i;
    rt_err_t ret;
    uint32_t max_try = 5;
    uint16_t prod_id;
    
    rt_adis.acc_ratio = 0.00025F;
    rt_adis.gyr_ratio = 0.005F;
    
    for(i=0; i<max_try; i++)
    {
        prod_id = adis16460_read_reg(PROD_ID);
        if(prod_id == 0x404C)
        {
            ADIS_TRACE("%s ok prod_id:0x%X\r\n", __FUNCTION__, prod_id);
                
            adis16460_write_register(MSC_CTRL, 0xC1);  // Enable Data Ready, set polarity
            rt_thread_delay(rt_tick_from_millisecond(10));
            
            adis16460_write_register(FLTR_CTRL, 0x500); // Set digital filter
            rt_thread_delay(rt_tick_from_millisecond(10));
            
            adis16460_write_register(DEC_RATE, 3), // Disable decimation
            rt_thread_delay(rt_tick_from_millisecond(10));

//            prod_id = adis16460_read_reg(MSC_CTRL);
//            ADIS_TRACE("0x%X\r\n", prod_id);
//                
//            prod_id = adis16460_read_reg(FLTR_CTRL);
//            ADIS_TRACE("0x%X\r\n", prod_id);
//                
//            prod_id = adis16460_read_reg(DEC_RATE);
//            ADIS_TRACE("0x%X\r\n", prod_id);
            
            rt_thread_t tid = rt_thread_create("adis", thread_adis_entry, RT_NULL, 512, 7, 20);
            rt_thread_startup(tid);
            
            /* enable raw sensor gyr interrupt */
            rt_pin_mode(rt_adis.int_pin, PIN_MODE_INPUT_PULLUP);
            rt_pin_attach_irq(rt_adis.int_pin, PIN_IRQ_MODE_RISING, adis_pin_int, RT_NULL);
            rt_pin_irq_enable(rt_adis.int_pin, PIN_IRQ_ENABLE);
            ret = RT_EOK;
            break;
        }
    }
    
    if(i == max_try)
    {
        ADIS_TRACE("%s failed prod_id:0x%X\r\n", __FUNCTION__, prod_id);
        ret = RT_EIO;
    }

    return ret;
}




static rt_size_t rt_adis16460_read(rt_device_t dev, rt_off_t pos, void* buffer, rt_size_t size)
{
    float *p = (float*)buffer;
    int16_t tmp[3];
    uint16_t cal_chksum;
    uint16_t chksum;
    static uint8_t chk_ok = 0;

    switch(pos)
    {
        case RT_SENSOR_POS_ACC:
            if(chk_ok)
            {
                tmp[0] = ((rt_adis.rx_buf[8]  << 8) | (rt_adis.rx_buf[9] & 0xFF));
                tmp[1] = ((rt_adis.rx_buf[10] << 8) | (rt_adis.rx_buf[11] & 0xFF));
                tmp[2] = ((rt_adis.rx_buf[12] << 8) | (rt_adis.rx_buf[13] & 0xFF));
            
                p[0] = tmp[0] * rt_adis.acc_ratio;
                p[1] = tmp[1] * rt_adis.acc_ratio;
                p[2] = tmp[2] * rt_adis.acc_ratio;

            }
            break;
        case RT_SENSOR_POS_GYR:
            rt_mutex_take(rt_adis.mtx, RT_WAITING_FOREVER);
        
            cal_chksum = _checksum(rt_adis.rx_buf);
            chksum = ((rt_adis.rx_buf[18] << 8) | (rt_adis.rx_buf[19]));
        
            (chksum == cal_chksum)?(chk_ok = 1):(chk_ok = 0);
        
            if(chk_ok)
            {
                tmp[0] = ((rt_adis.rx_buf[2] << 8) | (rt_adis.rx_buf[3] & 0xFF));
                tmp[1] = ((rt_adis.rx_buf[4] << 8) | (rt_adis.rx_buf[5] & 0xFF));
                tmp[2] = ((rt_adis.rx_buf[6] << 8) | (rt_adis.rx_buf[7] & 0xFF));
                
                p[0] = tmp[0] * rt_adis.gyr_ratio;
                p[1] = tmp[1] * rt_adis.gyr_ratio;
                p[2] = tmp[2] * rt_adis.gyr_ratio;
            }
            else
            {
                //ADIS_TRACE("chk error\r\n");
            }
            rt_mutex_release(rt_adis.mtx);
            
            break;
        case RT_SENSOR_POS_GYR_TEMPERATURE:
            tmp[0] = ((rt_adis.rx_buf[8] << 8) | (rt_adis.rx_buf[9] & 0xFF));
            p[0] = tmp[0] * 0.05F + 25;
            break;
    }
    return size;
}

static rt_size_t rt_adis16460_write(rt_device_t dev, rt_off_t pos, const void* buffer, rt_size_t size)
{   
    return size;
}

static rt_err_t rt_adis16460_control(rt_device_t dev, int cmd, void *args)
{
    return RT_EOK;
}

int rt_hw_adis16460_init(const char *name, uint8_t int_pin)
{
    struct rt_device *dev;
    
    dev                     = &rt_adis.parent;
	dev->type               = RT_Device_Class_Miscellaneous;
	dev->rx_indicate        = RT_NULL;
	dev->tx_complete        = RT_NULL;
	dev->init               = rt_adis16460_init;
	dev->open               = rt_adis16460_open;
	dev->close              = RT_NULL;
	dev->read               = rt_adis16460_read;
	dev->write              = rt_adis16460_write;
	dev->control            = rt_adis16460_control;
	dev->user_data          = RT_NULL;
 
    rt_adis.int_pin = int_pin;
    rt_adis.sem = rt_sem_create("adis", 0, RT_IPC_FLAG_FIFO);
    rt_adis.mtx = rt_mutex_create("adis", RT_IPC_FLAG_FIFO);
    rt_device_register(dev, name, RT_DEVICE_FLAG_RDWR);
    return RT_EOK;
}

/* adis pin interrupt */
static void adis_pin_int(void *args)
{
    rt_sem_release(rt_adis.sem);
    if(rt_adis.parent.rx_indicate)
    {
        rt_adis.parent.rx_indicate(&rt_adis.parent, 1<<RT_SENSOR_POS_ACC);
        rt_adis.parent.rx_indicate(&rt_adis.parent, 1<<RT_SENSOR_POS_GYR);
    }
}

