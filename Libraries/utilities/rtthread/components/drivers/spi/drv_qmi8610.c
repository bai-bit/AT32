#include <rtthread.h>
#include <stdio.h>
#include <rtdevice.h>

#include "drivers/rt_sensor.h"



#define QMI_DEBUG		1
#if ( QMI_DEBUG == 1 )
#include <stdio.h>
#define QMI_TRACE	printf
#else
#define QMI_TRACE(...)
#endif



#define FISIMU_CTRL7_DISABLE_ALL		(0x0)
#define FISIMU_CTRL7_ACC_ENABLE			(0x1)
#define FISIMU_CTRL7_GYR_ENABLE			(0x2)
#define FISIMU_CTRL7_MAG_ENABLE			(0x4)
#define FISIMU_CTRL7_AE_ENABLE			(0x8)
#define FISIMU_CTRL7_ENABLE_MASK		(0xF)

#define FISIMU_CONFIG_ACC_ENABLE		FISIMU_CTRL7_ACC_ENABLE
#define FISIMU_CONFIG_GYR_ENABLE		FISIMU_CTRL7_GYR_ENABLE
#define FISIMU_CONFIG_MAG_ENABLE		FISIMU_CTRL7_MAG_ENABLE
#define FISIMU_CONFIG_AE_ENABLE			FISIMU_CTRL7_AE_ENABLE
#define FISIMU_CONFIG_ACCGYR_ENABLE		(FISIMU_CONFIG_ACC_ENABLE | FISIMU_CONFIG_GYR_ENABLE)
#define FISIMU_CONFIG_ACCGYRMAG_ENABLE	(FISIMU_CONFIG_ACC_ENABLE | FISIMU_CONFIG_GYR_ENABLE | FISIMU_CONFIG_MAG_ENABLE)
#define FISIMU_CONFIG_AEMAG_ENABLE		(FISIMU_CONFIG_AE_ENABLE | FISIMU_CONFIG_MAG_ENABLE)

#define FISIMU_STATUS1_CMD_DONE			(0x01)
#define FISIMU_STATUS1_WAKEUP_EVENT		(0x04)


typedef struct
{
    struct                  rt_device parent;
    struct rt_spi_device    *spi_dev;
    uint32_t                irq_pin;        /* acc interrupt pin */
    uint32_t                rst_pin;
}rt_qmi_t;

static rt_qmi_t rt_qmi;


enum FIS210xRegister
{
	/*! \brief FIS device identifier register. */
	FisRegister_WhoAmI=0, // 0
	/*! \brief FIS hardware revision register. */
	FisRegister_Revision, // 1
	/*! \brief General and power management modes. */
	FisRegister_Ctrl1, // 2
	/*! \brief Accelerometer control. */
	FisRegister_Ctrl2, // 3
	/*! \brief Gyroscope control. */
	FisRegister_Ctrl3, // 4
	/*! \brief Magnetometer control. */
	FisRegister_Ctrl4, // 5
	/*! \brief Data processing settings. */
	FisRegister_Ctrl5, // 6
	/*! \brief AttitudeEngine control. */
	FisRegister_Ctrl6, // 7
	/*! \brief Sensor enabled status. */
	FisRegister_Ctrl7, // 8
	/*! \brief Reserved - do not write. */
	FisRegister_Ctrl8, // 9
	/*! \brief Host command register. */
	FisRegister_Ctrl9,
	/*! \brief Calibration register 1 least significant byte. */
	FisRegister_Cal1_L,
	/*! \brief Calibration register 1 most significant byte. */
	FisRegister_Cal1_H,
	/*! \brief Calibration register 2 least significant byte. */
	FisRegister_Cal2_L,
	/*! \brief Calibration register 2 most significant byte. */
	FisRegister_Cal2_H,
	/*! \brief Calibration register 3 least significant byte. */
	FisRegister_Cal3_L,
	/*! \brief Calibration register 3 most significant byte. */
	FisRegister_Cal3_H,
	/*! \brief Calibration register 4 least significant byte. */
	FisRegister_Cal4_L,
	/*! \brief Calibration register 4 most significant byte. */
	FisRegister_Cal4_H,
	/*! \brief FIFO control register. */
	FisRegister_FifoCtrl,
	/*! \brief FIFO data register. */
	FisRegister_FifoData,
	/*! \brief FIFO status register. */
	FisRegister_FifoStatus,
	/*! \brief Output data overrun and availability. */
	FisRegister_Status0,
	/*! \brief Miscellaneous status register. */
	FisRegister_Status1,
	/*! \brief Sample counter. */
	FisRegister_CountOut,
	/*! \brief Accelerometer X axis least significant byte. */
	FisRegister_Ax_L,
	/*! \brief Accelerometer X axis most significant byte. */
	FisRegister_Ax_H,
	/*! \brief Accelerometer Y axis least significant byte. */
	FisRegister_Ay_L,
	/*! \brief Accelerometer Y axis most significant byte. */
	FisRegister_Ay_H,
	/*! \brief Accelerometer Z axis least significant byte. */
	FisRegister_Az_L,
	/*! \brief Accelerometer Z axis most significant byte. */
	FisRegister_Az_H,
	/*! \brief Gyroscope X axis least significant byte. */
	FisRegister_Gx_L,
	/*! \brief Gyroscope X axis most significant byte. */
	FisRegister_Gx_H,
	/*! \brief Gyroscope Y axis least significant byte. */
	FisRegister_Gy_L,
	/*! \brief Gyroscope Y axis most significant byte. */
	FisRegister_Gy_H,
	/*! \brief Gyroscope Z axis least significant byte. */
	FisRegister_Gz_L,
	/*! \brief Gyroscope Z axis most significant byte. */
	FisRegister_Gz_H,
	/*! \brief Magnetometer X axis least significant byte. */
	FisRegister_Mx_L,
	/*! \brief Magnetometer X axis most significant byte. */
	FisRegister_Mx_H,
	/*! \brief Magnetometer Y axis least significant byte. */
	FisRegister_My_L,
	/*! \brief Magnetometer Y axis most significant byte. */
	FisRegister_My_H,
	/*! \brief Magnetometer Z axis least significant byte. */
	FisRegister_Mz_L,
	/*! \brief Magnetometer Z axis most significant byte. */
	FisRegister_Mz_H,
	/*! \brief Quaternion increment W least significant byte. */
	FisRegister_Q1_L = 45,
	/*! \brief Quaternion increment W most significant byte. */
	FisRegister_Q1_H,
	/*! \brief Quaternion increment X least significant byte. */
	FisRegister_Q2_L,
	/*! \brief Quaternion increment X most significant byte. */
	FisRegister_Q2_H,
	/*! \brief Quaternion increment Y least significant byte. */
	FisRegister_Q3_L,
	/*! \brief Quaternion increment Y most significant byte. */
	FisRegister_Q3_H,
	/*! \brief Quaternion increment Z least significant byte. */
	FisRegister_Q4_L,
	/*! \brief Quaternion increment Z most significant byte. */
	FisRegister_Q4_H,
	/*! \brief Velocity increment X least significant byte. */
	FisRegister_Dvx_L,
	/*! \brief Velocity increment X most significant byte. */
	FisRegister_Dvx_H,
	/*! \brief Velocity increment Y least significant byte. */
	FisRegister_Dvy_L,
	/*! \brief Velocity increment Y most significant byte. */
	FisRegister_Dvy_H,
	/*! \brief Velocity increment Z least significant byte. */
	FisRegister_Dvz_L,
	/*! \brief Velocity increment Z most significant byte. */
	FisRegister_Dvz_H,
	/*! \brief Temperature output. */
	FisRegister_Temperature,
	/*! \brief AttitudeEngine clipping flags. */
	FisRegister_AeClipping,
	/*! \brief AttitudeEngine overflow flags. */
	FisRegister_AeOverflow,
};

enum FisImu_Ctrl9Command
{
	/*! \brief No operation. */
	Ctrl9_Nop = 0,
	/*! \brief Reset FIFO. */
	Ctrl9_ResetFifo = 0x2,
	/*! \brief Set magnetometer X calibration values. */
	Ctrl9_SetMagXCalibration = 0x6,
	/*! \brief Set magnetometer Y calibration values. */
	Ctrl9_SetMagYCalibration = 0x7,
	/*! \brief Set magnetometer Z calibration values. */
	Ctrl9_SetMagZCalibration = 0x8,
	/*! \brief Set accelerometer offset correction value. */
	Ctrl9_SetAccelOffset = 0x12,
	/*! \brief Set gyroscope offset correction value. */
	Ctrl9_SetGyroOffset = 0x13,
	/*! \brief Set accelerometer sensitivity. */
	Ctrl9_SetAccelSensitivity = 0x14,
	/*! \brief Set gyroscope sensitivity. */
	Ctrl9_SetGyroSensitivity = 0x15,
	/*! \brief Update magnemoter bias compensation. */
	Ctrl9_UpdateMagBias = 0xB,
	/*! \brief Trigger motion on demand sample. */
	Ctrl9_TriggerMotionOnDemand = 0x0c,
	/*! \brief Update gyroscope bias compensation. */
	Ctrl9_UpdateAttitudeEngineGyroBias = 0xE,
	/*! \brief Read frequency correction value. */
	Ctrl9_ReadTrimmedFrequencyValue = 0x18,
	/*! \brief Prepare for FIFO read sequence. */
	Ctrl9_ReadFifo = 0x0D,
	/*! \brief Set wake on motion parameters. */
	Ctrl9_ConfigureWakeOnMotion = 0x19,
};


static void qmi_write_register(uint8_t addr, uint8_t val)
{
    uint8_t buf[2];
    
    buf[0] = (0<<7) | addr;
    buf[1] = val;
    
    rt_spi_send(rt_qmi.spi_dev, buf, 2);
}

static uint8_t qmi_read_register(uint8_t addr)
{
    uint8_t send[1], recv[1];
    
    send[0] = (1<<7) | addr;
    
    rt_spi_send_then_recv(rt_qmi.spi_dev, send, 1, recv, 1);
    return recv[0];
}

static uint8_t qmi_read(uint8_t addr, void *buf, uint32_t len)
{
    uint8_t send[1];
    
    send[0] = (1<<7) | (1<<6) | addr;
    
    return rt_spi_send_then_recv(rt_qmi.spi_dev, send, 1, buf, len);
}

static rt_err_t rt_qmi_init(rt_device_t dev)
{    
    return RT_EOK;
}


static rt_err_t rt_qmi_open(rt_device_t dev, rt_uint16_t oflag)
{
    uint32_t i;
    uint8_t val = 0;
    int ret;
    
    rt_pin_mode(rt_qmi.rst_pin, PIN_MODE_OUTPUT);
    rt_pin_write(rt_qmi.rst_pin, 1);
    rt_thread_delay(1);
    rt_pin_write(rt_qmi.rst_pin, 0);
    rt_thread_delay(2000);
    
    val = qmi_read_register(FisRegister_WhoAmI);
    
    if(val == 0xFC)
    {
        QMI_TRACE("QMI found\r\n");
        
        qmi_write_register(FisRegister_Ctrl7, 0x00);    /* disable all */
        qmi_write_register(FisRegister_Ctrl2, (2<<3)); /* acc 8G */
        qmi_write_register(FisRegister_Ctrl3, (6<<3) | (1<<0)); /* gyr: 2048 dps LPF opt:1 */
        qmi_write_register(FisRegister_Ctrl5, (1<<1) | (2<<3)); /* LPF enable*/
        
        qmi_write_register(FisRegister_Ctrl7, 0x03); /* enable acc & gyr */
        
        /* dump register to check */
        for(i=0; i<10; i++)
        {
            QMI_TRACE("REG:%d:0x%X\r\n", i, qmi_read_register(i));
        }

        ret = RT_EOK;
    }
    else
    {
        QMI_TRACE("QMI not found 0x%X\r\n", val);
        ret = RT_EIO;
    }
    
    
    while(1)
    {
        int16_t acc[3];
        int16_t gyr[3];
        float fgyr[3];
        qmi_read(FisRegister_Ax_L, acc, 6);
        qmi_read(FisRegister_Gx_L, gyr, 6);
       // printf("%d %d %d\r\n", acc[0], acc[1], acc[2]);
        
        float gyr_ratio = (2048.0F/32768.0F);
        static float gyr_sum[3];
        
        fgyr[0] = (float)gyr[0] * gyr_ratio;
        fgyr[1] = (float)gyr[1] * gyr_ratio;
        fgyr[2] = (float)gyr[2] * gyr_ratio;
        
        
        gyr_sum[0] += fgyr[0]/10;
        gyr_sum[1] += fgyr[1]/10;
        gyr_sum[2] += fgyr[2]/10;
        
      //  printf("%d %d %d\r\n", gyr[0], gyr[1], gyr[2]);
        printf("%8.3f, %8.3f, %8.3f\r\n", fgyr[0], fgyr[1], fgyr[2]);
      //  printf("%8.3f %8.3f %8.3f\r\n", gyr_sum[0], gyr_sum[1], gyr_sum[2]);
        rt_thread_delay(100);
        
    }
    return ret;
}


static rt_size_t rt_qmi_read(rt_device_t dev, rt_off_t pos, void* buffer, rt_size_t size)
{
    return size;
}

static rt_size_t rt_qmi_write(rt_device_t dev, rt_off_t pos, const void* buffer, rt_size_t size)
{
    return size;
}

static rt_err_t rt_qmi_control(rt_device_t dev, int cmd, void *args)
{
    
    return RT_EOK;
}


int rt_hw_qmi8610_init(const char *name, const char *spid_name, uint32_t irq_pin, uint32_t rst_pin)
{
    rt_qmi.irq_pin = irq_pin;
    rt_qmi.rst_pin = rst_pin;
    rt_qmi.spi_dev = (struct rt_spi_device *)rt_device_find(spid_name);
    
    if(!rt_qmi.spi_dev)
    {
        return RT_ENOSYS;
    }
    
	rt_qmi.parent.type               = RT_Device_Class_Miscellaneous;
	rt_qmi.parent.rx_indicate        = RT_NULL;
	rt_qmi.parent.tx_complete        = RT_NULL;
	rt_qmi.parent.init               = rt_qmi_init;
	rt_qmi.parent.open               = rt_qmi_open;
	rt_qmi.parent.close              = RT_NULL;
	rt_qmi.parent.read               = rt_qmi_read;
	rt_qmi.parent.write              = rt_qmi_write;
	rt_qmi.parent.control            = rt_qmi_control;
	rt_qmi.parent.user_data          = &rt_qmi;
 
    rt_device_register(&rt_qmi.parent, name, RT_DEVICE_FLAG_RDWR);
    return RT_EOK;
}


