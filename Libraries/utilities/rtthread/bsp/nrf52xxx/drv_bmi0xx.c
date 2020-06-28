#include <rtthread.h>
#include <stdio.h>
#include <rtdevice.h>
#include "common.h"
#include "spi.h"
#include "gpio.h"
#include "drivers/rt_sensor.h"
#include "cm_math.h"


#define BMI0XX_DEBUG		0
#if ( BMI0XX_DEBUG == 1 )
#include <stdio.h>
#define BMI0XX_TRACE	printf
#else
#define BMI0XX_TRACE(...)
#endif

/* BMI055 BMI088 GYR */
#define BMG160_CHIP_ID_ADDR                 (0x00)
#define BMG160_RATE_X_LSB_ADDR              (0x02)
#define BMG160_RATE_X_MSB_ADDR              (0x03)
#define BMG160_RATE_Y_LSB_ADDR              (0x04)
#define BMG160_RATE_Y_MSB_ADDR              (0x05)
#define BMG160_RATE_Z_LSB_ADDR              (0x06)
#define BMG160_RATE_Z_MSB_ADDR              (0x07)
#define BMG160_TEMP_ADDR                    (0x08)
#define BMG160_INTR_STAT0_ADDR              (0x09)
#define BMG160_INTR_STAT1_ADDR              (0x0A)
#define BMG160_INTR_STAT2_ADDR              (0x0B)
#define BMG160_INTR_STAT3_ADDR              (0x0C)
#define BMG160_FIFO_STAT_ADDR               (0x0E)
#define BMG160_RANGE_ADDR                   (0x0F)
#define BMG160_BW_ADDR                      (0x10)
#define BMG160_MODE_LPM1_ADDR               (0x11)
#define BMG160_MODE_LPM2_ADDR               (0x12)
#define BMG160_HIGH_BW_ADDR                 (0x13)
#define BMG160_BGW_SOFT_RST_ADDR            (0x14)
#define BMG160_INTR_ENABLE0_ADDR            (0x15)
#define BMG160_INTR_ENABLE1_ADDR            (0x16)
#define BMG160_INTR_MAP_ZERO_ADDR           (0x17)
#define BMG160_INTR_MAP_ONE_ADDR            (0x18)
#define BMG160_INTR_MAP_TWO_ADDR            (0x19)
#define BMG160_INTR_ZERO_ADDR               (0x1A)
#define BMG160_INTR_ONE_ADDR                (0x1B)
#define BMG160_INTR_TWO_ADDR                (0x1C)
#define BMG160_INTR_4_ADDR                  (0x1E)
#define BMG160_RST_LATCH_ADDR               (0x21)
#define BMG160_HIGHRATE_THRES_X_ADDR        (0x22)
#define BMG160_HIGHRATE_DURN_X_ADDR         (0x23)
#define BMG160_HIGHRATE_THRES_Y_ADDR        (0x24)
#define BMG160_HIGHRATE_DURN_Y_ADDR         (0x25)
#define BMG160_HIGHRATE_THRES_Z_ADDR        (0x26)
#define BMG160_HIGHRATE_DURN_Z_ADDR         (0x27)
#define BMG160_SOC_ADDR                     (0x31)
#define BMG160_A_FOC_ADDR                   (0x32)
#define BMG160_TRIM_NVM_CTRL_ADDR           (0x33)
#define BMG160_BGW_SPI3_WDT_ADDR            (0x34)
#define BMG160_OFFSET_OFC1_ADDR             (0x36)
#define BMG160_OFC2_ADDR                    (0x37)
#define BMG160_OFC3_ADDR                    (0x38)
#define BMG160_OFC4_ADDR                    (0x39)
#define BMG160_TRIM_GP0_ADDR                (0x3A)
#define BMG160_TRIM_GP1_ADDR                (0x3B)
#define BMG160_SELFTEST_ADDR                (0x3C)
#define BMG160_FIFO_CGF1_ADDR               (0x3E)
#define BMG160_FIFO_CGF0_ADDR               (0x3D)
#define BMG160_FIFO_DATA_ADDR               (0x3F)

/* BMI055 ACC */
#define BMA2x2_EEP_OFFSET                   (0x16)
#define BMA2x2_IMAGE_BASE                   (0x38)
#define BMA2x2_IMAGE_LEN                    (22)
#define BMA2x2_CHIP_ID_ADDR                 (0x00)
#define BMA2x2_X_AXIS_LSB_ADDR              (0x02)
#define BMA2x2_X_AXIS_MSB_ADDR              (0x03)
#define BMA2x2_Y_AXIS_LSB_ADDR              (0x04)
#define BMA2x2_Y_AXIS_MSB_ADDR              (0x05)
#define BMA2x2_Z_AXIS_LSB_ADDR              (0x06)
#define BMA2x2_Z_AXIS_MSB_ADDR              (0x07)
#define BMA2x2_TEMP_ADDR                    (0x08)
#define BMA2x2_STAT1_ADDR                   (0x09)
#define BMA2x2_STAT2_ADDR                   (0x0A)
#define BMA2x2_STAT_TAP_SLOPE_ADDR          (0x0B)
#define BMA2x2_STAT_ORIENT_HIGH_ADDR        (0x0C)
#define BMA2x2_STAT_FIFO_ADDR               (0x0E)
#define BMA2x2_RANGE_SELECT_ADDR            (0x0F)
#define BMA2x2_BW_SELECT_ADDR               (0x10)
#define BMA2x2_MODE_CTRL_ADDR               (0x11)
#define BMA2x2_LOW_NOISE_CTRL_ADDR          (0x12)
#define BMA2x2_DATA_CTRL_ADDR               (0x13)
#define BMA2x2_RST_ADDR                     (0x14)
#define BMA2x2_INTR_ENABLE0_ADDR            (0x16)
#define BMA2x2_INTR_ENABLE1_ADDR            (0x17)
#define BMA2x2_INTR_ENABLE2_ADDR            (0x18)
#define BMA2x2_INTR_SLOW_NO_MOTION_ADDR     (0x18)
#define BMA2x2_INTR1_PAD_SELECT_ADDR        (0x19)
#define BMA2x2_INTR_DATA_SELECT_ADDR        (0x1A)
#define BMA2x2_INTR2_PAD_SELECT_ADDR        (0x1B)
#define BMA2x2_INTR_SOURCE_ADDR             (0x1E)
#define BMA2x2_INTR_SET_ADDR                (0x20)
#define BMA2x2_INTR_CTRL_ADDR               (0x21)
#define BMA2x2_LOW_DURN_ADDR                (0x22)
#define BMA2x2_LOW_THRES_ADDR               (0x23)
#define BMA2x2_LOW_HIGH_HYST_ADDR           (0x24)
#define BMA2x2_HIGH_DURN_ADDR               (0x25)
#define BMA2x2_HIGH_THRES_ADDR              (0x26)
#define BMA2x2_SLOPE_DURN_ADDR              (0x27)
#define BMA2x2_SLOPE_THRES_ADDR             (0x28)
#define BMA2x2_SLOW_NO_MOTION_THRES_ADDR    (0x29)
#define BMA2x2_TAP_PARAM_ADDR               (0x2A)
#define BMA2x2_TAP_THRES_ADDR               (0x2B)
#define BMA2x2_ORIENT_PARAM_ADDR            (0x2C)
#define BMA2x2_THETA_BLOCK_ADDR             (0x2D)
#define BMA2x2_THETA_FLAT_ADDR              (0x2E)
#define BMA2x2_FLAT_HOLD_TIME_ADDR          (0x2F)
#define BMA2x2_SELFTEST_ADDR                (0x32)
#define BMA2x2_EEPROM_CTRL_ADDR             (0x33)
#define BMA2x2_SERIAL_CTRL_ADDR             (0x34)
#define BMA2x2_OFFSET_CTRL_ADDR             (0x36)
#define BMA2x2_OFFSET_PARAMS_ADDR           (0x37)
#define BMA2x2_OFFSET_X_AXIS_ADDR           (0x38)
#define BMA2x2_OFFSET_Y_AXIS_ADDR           (0x39)
#define BMA2x2_OFFSET_Z_AXIS_ADDR           (0x3A)
#define BMA2x2_GP0_ADDR                     (0x3B)
#define BMA2x2_GP1_ADDR                     (0x3C)
#define BMA2x2_FIFO_MODE_ADDR               (0x3E)
#define BMA2x2_FIFO_DATA_OUTPUT_ADDR        (0x3F)
#define BMA2x2_FIFO_WML_TRIG                (0x30)

/* BMI088 ACC */
#define BMI08X_ACC_CHIP_ID_ADDR             (0x00)
#define BMI08X_ACC_CONF                     (0x40)
#define BMI08X_ACC_RANGE                    (0x41)
#define BMI08X_ACC_FIFO_CFG0                (0x48)
#define BMI08X_ACC_FIFO_CFG1                (0x49)
#define BMI08X_INT1_INT2_MAP_DATA           (0x58)
#define BMI08X_FIFO_WTM0                    (0x46)
#define BMI08X_FIFO_WTM1                    (0x47)
#define BMI08X_ACC_X_AXIS_LSB_ADDR          (0x12)
#define BMI08X_ACC_TEMP_ADDR                (0x22)
#define BMI08X_ACC_FIFO_DATA                (0x26)
#define BMI088_ACC_CHIP_ID                  (0x1E)
#define BMI055_ACC_CHIP_ID                  (0xFA)

#define BMI_GYR_CS                          (0)
#define BMI_ACC_CS                          (1)

static void bmi0xx_gyr_pin_int_handler(void *args);
static void bmi0xx_acc_pin_int_handler(void *args);


typedef struct
{
    struct          rt_device parent;
    uint8_t         rx_gyr_dma_buf[6*20];   /* async rx buf */
    uint8_t         rx_acc_dma_buf[6*20];
    uint32_t        acc_rg_tb[4];
    uint32_t        acc_bw_tb[7];
    uint32_t        gyr_rg_tb[4];
    uint32_t        gyr_bw_tb[7];
    uint32_t        acc_rg;
    uint32_t        acc_bw;
    uint32_t        gyr_rg;
    uint32_t        gyr_bw;
    uint32_t        acc_int_pin;
    uint32_t        gyr_int_pin;
    uint8_t         acc_chip_id;
    uint8_t         gyr_chip_id;
    uint8_t         cs;             /* current chip select */
    uint8_t         gyr_fifo_read_size;
    uint8_t         acc_fifo_read_size;
    rt_mutex_t      mtx;
    cm_type_t       acc_ratio;  /* ratio for G */
    cm_type_t       gyr_ratio;  /* ratio for DPS */
}rt_bmi_t;



static rt_bmi_t rt_bmi;


static void bmi0xx_set_cs(uint32_t chip, uint32_t state)
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

static void bmi0xx_write_register(uint8_t cs, uint8_t addr, uint8_t val)
{
    rt_mutex_take(rt_bmi.mtx, RT_WAITING_FOREVER);
    bmi0xx_set_cs(cs, 0);
    
    uint8_t tx_dat[2];
    tx_dat[0] = (0<<7) | addr;
    tx_dat[1] = val;
    
    SPI_ReadWriteFIFO(HW_SPI0, tx_dat, NULL, 2);
    
    bmi0xx_set_cs(cs, 1);
    rt_mutex_release(rt_bmi.mtx);
}

static uint8_t bmi0xx_read_register(uint8_t cs, uint8_t addr)
{
    uint8_t val;
    rt_mutex_take(rt_bmi.mtx, RT_WAITING_FOREVER);
    bmi0xx_set_cs(cs, 0);
    
    SPI_ReadWrite(HW_SPI0, (1<<7) | addr);
    val = SPI_ReadWrite(HW_SPI0, 0xFF);
    
    bmi0xx_set_cs(cs, 1);
    rt_mutex_release(rt_bmi.mtx);
    return val;
}

static void bmi0xx_read(uint8_t cs, uint8_t addr, uint8_t *rx_buf, uint32_t len)
{
  //  int i;
    bmi0xx_set_cs(cs, 0);
    
    SPI_ReadWrite(HW_SPI0, (1<<7) | addr);
    
    SPI_ReadWriteFIFO(HW_SPI0, NULL, rx_buf, len);
//    for(i=0; i<len; i++)
//    {
//        rx_buf[i] = SPI_ReadWrite(HW_SPI0, 0xFF);
//    }
    
    bmi0xx_set_cs(cs, 1);
}


static rt_err_t rt_bmi0xx_init(rt_device_t dev)
{    
    SPI_Init(HW_SPI0, 8*1000*1000);
    
    GPIO_Init(HW_GPIOA, 14, kGPIO_OPPH); /* ACC CS 0xFA */
    GPIO_Init(HW_GPIOA, 16, kGPIO_OPPH); /* GYR CS 0x0F */
    
    
    NRF_SPIM0->PSEL.MISO = 15;
    NRF_SPIM0->PSEL.MOSI = 13;
    NRF_SPIM0->PSEL.SCK = 12;
    
    
    return RT_EOK;
}


static rt_err_t rt_bmi055_open(rt_device_t dev, rt_uint16_t oflag)
{
    rt_err_t ret = 0;
    
    /* probe gyr (BMG160) */
    rt_bmi.gyr_chip_id = bmi0xx_read_register(BMI_GYR_CS, BMG160_CHIP_ID_ADDR);
    
    if(rt_bmi.gyr_chip_id == 0x0F)
    {
        ret++;
        BMI0XX_TRACE("BMI0xx gyr found addr:0x%02X\r\n", rt_bmi.gyr_chip_id);
        
        /* load attr table */
        rt_bmi.gyr_rg_tb[0] = 250;
        rt_bmi.gyr_rg_tb[1] = 500;
        rt_bmi.gyr_rg_tb[2] = 1000;
        rt_bmi.gyr_rg_tb[3] = 2000;
        
        rt_bmi.gyr_bw_tb[0] = 12;
        rt_bmi.gyr_bw_tb[1] = 23;
        rt_bmi.gyr_bw_tb[2] = 32;
        rt_bmi.gyr_bw_tb[3] = 47;
        rt_bmi.gyr_bw_tb[4] = 64;
        rt_bmi.gyr_bw_tb[5] = 116;
        rt_bmi.gyr_bw_tb[6] = 230;
        
        /* reset gyr */
        bmi0xx_write_register(BMI_GYR_CS, BMG160_MODE_LPM1_ADDR, 0x20);
        bmi0xx_write_register(BMI_GYR_CS, BMG160_MODE_LPM1_ADDR, 0x00);
        bmi0xx_write_register(BMI_GYR_CS, BMG160_BGW_SOFT_RST_ADDR, 0xB6);
        rt_thread_delay(rt_tick_from_millisecond(20));
        
        /* set gyr bw 47Hz */
        bmi0xx_write_register(BMI_GYR_CS, BMG160_BW_ADDR, 0x03);
        
        /* set gyr rg 2000DPS */
        bmi0xx_write_register(BMI_GYR_CS, BMG160_RANGE_ADDR, 0x00);
        rt_bmi.gyr_ratio = (2000.0F/32768.0F);
        
    }
    else
    {
        BMI0XX_TRACE("BMI0xx gyr not found addr:0x%02X\r\n", rt_bmi.gyr_chip_id);
    }
    
    /* probe BMI088 ACC( BMI088 ACC need read twice) */
    rt_bmi.acc_chip_id = bmi0xx_read_register(BMI_ACC_CS, BMI08X_ACC_CHIP_ID_ADDR);
    rt_bmi.acc_chip_id = bmi0xx_read_register(BMI_ACC_CS, BMI08X_ACC_CHIP_ID_ADDR);
    
    if(rt_bmi.acc_chip_id == BMI088_ACC_CHIP_ID)
    {
        ret++;
        BMI0XX_TRACE("BMI088 acc found addr:0x%02X\r\n", rt_bmi.acc_chip_id);
        rt_bmi.acc_rg_tb[0] = 3;
        rt_bmi.acc_rg_tb[1] = 6;
        rt_bmi.acc_rg_tb[2] = 12;
        rt_bmi.acc_rg_tb[3] = 24;
        
        bmi0xx_write_register(BMI_ACC_CS, 0x7D, 0x04);  /* power up */
        rt_thread_delay(rt_tick_from_millisecond(20));
        bmi0xx_write_register(BMI_ACC_CS, 0x7C, 0x00);  /* acc enable */
        rt_thread_delay(rt_tick_from_millisecond(20));
        
        /* set acc to 62.5Hz */
        bmi0xx_write_register(BMI_ACC_CS, BMI08X_ACC_CONF, 0xA8);
        rt_thread_delay(rt_tick_from_millisecond(10));
        bmi0xx_read_register(BMI_ACC_CS, BMI08X_ACC_CONF);
        bmi0xx_read_register(BMI_ACC_CS, BMI08X_ACC_CONF);
        
        /* set acc to 6G */
        bmi0xx_write_register(BMI_ACC_CS, BMI08X_ACC_RANGE, 0x01);
        bmi0xx_read_register(BMI_ACC_CS, BMI08X_ACC_RANGE);
        bmi0xx_read_register(BMI_ACC_CS, BMI08X_ACC_RANGE);
        
        rt_bmi.acc_ratio = (6.0F/32768.0F);
    }
    else
    {
        BMI0XX_TRACE("BMI088 acc not found addr:0x%02X\r\n", rt_bmi.acc_chip_id);
    }
    
    /* probe bmi055 acc */
    rt_bmi.acc_chip_id = bmi0xx_read_register(BMI_ACC_CS, 0x00);
    if(rt_bmi.acc_chip_id == BMI055_ACC_CHIP_ID)
    {
        /* load BMI055 acc bw table */
        rt_bmi.acc_bw_tb[0] = 8;
        rt_bmi.acc_bw_tb[1] = 16;
        rt_bmi.acc_bw_tb[2] = 32;
        rt_bmi.acc_bw_tb[3] = 62;
        rt_bmi.acc_bw_tb[4] = 125;
        rt_bmi.acc_bw_tb[5] = 250;
        rt_bmi.acc_bw_tb[6] = 500;
        
        ret++;
        BMI0XX_TRACE("BMI055 acc found addr:0x%02X\r\n", rt_bmi.acc_chip_id);
        rt_bmi.acc_rg_tb[0] = 2;
        rt_bmi.acc_rg_tb[1] = 4;
        rt_bmi.acc_rg_tb[2] = 8;
        rt_bmi.acc_rg_tb[3] = 16;
        
        /* set acc rg 8G */
        bmi0xx_write_register(BMI_ACC_CS, BMA2x2_RANGE_SELECT_ADDR, 0x08);
        rt_bmi.acc_ratio = (8.0F/32768.0F);
        
//        /* set acc to bw 62.5Hz */
//        bmi0xx_write_register(BMI_ACC_CS, BMA2x2_BW_SELECT_ADDR, 0x0B);
        
        /* set acc to bw 250Hz */
        bmi0xx_write_register(BMI_ACC_CS, BMA2x2_BW_SELECT_ADDR, 0x0D);
    }
    else
    {
        BMI0XX_TRACE("BMI055 acc not found addr:0x%02X\r\n", rt_bmi.acc_chip_id);
    }
          
    if(ret == 2)
    {
//#if (APP_PN != 316)
//        rt_thread_t tid = rt_thread_create("bmi0xx", thread_bmi0xx_entry, RT_NULL, 512, 7, 20);
//        rt_thread_startup(tid);
//#endif
        return RT_EOK;
    }
    else
    {
        return RT_EIO;
    }
}


static rt_size_t rt_bmi0xx_read(rt_device_t dev, rt_off_t pos, void* buffer, rt_size_t size)
{
    int i;
    cm_type_t *buf = buffer;
    int16_t *p = RT_NULL;
 
    switch(pos)
    {
        case RT_SENSOR_POS_ACC:
            if(rt_bmi.acc_chip_id == BMI055_ACC_CHIP_ID)
            {
                bmi0xx_read(BMI_ACC_CS, BMA2x2_X_AXIS_LSB_ADDR, rt_bmi.rx_acc_dma_buf, 6);
                p = (int16_t*)&rt_bmi.rx_acc_dma_buf[0];
            }
            else
            {
                bmi0xx_read(BMI_ACC_CS, BMI08X_ACC_X_AXIS_LSB_ADDR, rt_bmi.rx_acc_dma_buf, 7);
                p = (int16_t*)&rt_bmi.rx_acc_dma_buf[1];
            }

            buf[0] = (cm_type_t)p[0] * rt_bmi.acc_ratio;
            buf[1] = (cm_type_t)p[1] * rt_bmi.acc_ratio;
            buf[2] = (cm_type_t)p[2] * rt_bmi.acc_ratio;
            break;
        case RT_SENSOR_POS_GYR:
            bmi0xx_read(BMI_GYR_CS, BMG160_RATE_X_LSB_ADDR, rt_bmi.rx_gyr_dma_buf, 6);
        
            p = (int16_t*)&rt_bmi.rx_gyr_dma_buf[0];
        
            buf[0] = rt_bmi.gyr_ratio * (cm_type_t)p[0];
            buf[1] = rt_bmi.gyr_ratio * (cm_type_t)p[1];
            buf[2] = rt_bmi.gyr_ratio * (cm_type_t)p[2];
            break;
        case RT_SENSOR_POS_ACC_FIFO:
            rt_bmi.acc_fifo_read_size = size;
            
            if(rt_bmi.acc_chip_id == BMI055_ACC_CHIP_ID)
            {
                p = (int16_t*)&rt_bmi.rx_acc_dma_buf[0];
                bmi0xx_read(BMI_ACC_CS, BMA2x2_FIFO_DATA_OUTPUT_ADDR, rt_bmi.rx_acc_dma_buf, 6*rt_bmi.acc_fifo_read_size);
                
                for(i=0; i<size; i++)
                {
                    buf[i*3+0] = rt_bmi.acc_ratio * p[i*3+0];
                    buf[i*3+1] = rt_bmi.acc_ratio * p[i*3+1];
                    buf[i*3+2] = rt_bmi.acc_ratio * p[i*3+2];
                }
            }
            else
            {
                bmi0xx_read(BMI_ACC_CS, BMI08X_ACC_FIFO_DATA, rt_bmi.rx_acc_dma_buf, 7*rt_bmi.acc_fifo_read_size+1);
                for(i=0; i<size; i++)
                {
                    p = (int16_t*)&rt_bmi.rx_acc_dma_buf[i*7+2];
                    buf[i*3+0] = rt_bmi.acc_ratio * p[0];
                    buf[i*3+1] = rt_bmi.acc_ratio * p[1];
                    buf[i*3+2] = rt_bmi.acc_ratio * p[2];
                }
            }
            
            break;
        case RT_SENSOR_POS_GYR_FIFO:
            rt_bmi.gyr_fifo_read_size = size;
            bmi0xx_read(BMI_GYR_CS, BMG160_FIFO_DATA_ADDR, rt_bmi.rx_gyr_dma_buf, 6*rt_bmi.gyr_fifo_read_size);
        
            p = (int16_t*)&rt_bmi.rx_gyr_dma_buf[0];
            for(i=0; i<size; i++)
            {
                buf[i*3+0] = rt_bmi.gyr_ratio * p[i*3+0];
                buf[i*3+1] = rt_bmi.gyr_ratio * p[i*3+1];
                buf[i*3+2] = rt_bmi.gyr_ratio * p[i*3+2];
            }
            break;
        case RT_SENSOR_POS_GYR_TEMPERATURE:
//            bmi055_read_acc_temperature(buffer);
            break;
        case RT_SENSOR_POS_ACC_BW:
            rt_memcpy(buffer, &rt_bmi.acc_bw_tb[rt_bmi.acc_bw], 4);
            break;
        case RT_SENSOR_POS_ACC_RG:
            rt_memcpy(buffer, &rt_bmi.acc_rg_tb[rt_bmi.acc_rg], 4);
            break;
        case RT_SENSOR_POS_GYR_RG:
            rt_memcpy(buffer, &rt_bmi.gyr_rg_tb[rt_bmi.gyr_rg], 4);
            break;
        case RT_SENSOR_POS_GYR_BW:
            rt_memcpy(buffer, &rt_bmi.gyr_bw_tb[rt_bmi.gyr_bw], 4);
            break;
    }
    return size;
}


static rt_size_t rt_bmi055_write(rt_device_t dev, rt_off_t pos, const void* buffer, rt_size_t size)
{
    uint8_t *val = (uint8_t *)buffer;
    
    switch(pos)
    {
        case RT_SENSOR_POS_ACC_CLEAR_FIFO:
            if(rt_bmi.acc_chip_id == BMI055_ACC_CHIP_ID)
            {
                bmi0xx_write_register(BMI_ACC_CS, BMA2x2_FIFO_MODE_ADDR, 0x4C); 
            }
            else
            {
                bmi0xx_write_register(BMI_ACC_CS, 0x7E, 0xB0); 
            }
            break;
        case RT_SENSOR_POS_GYR_CLEAR_FIFO:
            bmi0xx_write_register(BMI_GYR_CS, BMG160_FIFO_CGF1_ADDR, 0x8C); 
            break;
        case RT_SENSOR_POS_ACC_WM_FIFO_INT:
            if(rt_bmi.acc_chip_id == BMI055_ACC_CHIP_ID)
            {
    //            bmi0xx_write_register(BMI_ACC_CS, BMA2x2_FIFO_MODE_ADDR,            0x00); /* FIFO mode : BYPASS mode */
                
                bmi0xx_write_register(BMI_ACC_CS, BMA2x2_INTR_SET_ADDR,             0x05);                          /* push pull, active high for INT1 & INT2 */
                bmi0xx_write_register(BMI_ACC_CS, 0x30,                             rt_bmi.acc_fifo_read_size);     /* fifo watermark: 1 frame */
                bmi0xx_write_register(BMI_ACC_CS, BMA2x2_FIFO_MODE_ADDR,            0x4C);                          /* FIFO mode : FIFO mode */
                bmi0xx_write_register(BMI_ACC_CS, BMA2x2_TAP_PARAM_ADDR,            0x04);                          /* TAP control */
                bmi0xx_write_register(BMI_ACC_CS, BMA2x2_TAP_THRES_ADDR,            0x01);
                bmi0xx_write_register(BMI_ACC_CS, BMA2x2_INTR_SOURCE_ADDR,          0x00);                          /* disable all un-related interrupts */
                bmi0xx_write_register(BMI_ACC_CS, BMA2x2_MODE_CTRL_ADDR,            0x00);
                bmi0xx_write_register(BMI_ACC_CS, BMA2x2_LOW_NOISE_CTRL_ADDR,       0x00);
                bmi0xx_write_register(BMI_ACC_CS, BMA2x2_DATA_CTRL_ADDR,            0x00);                          /* select un-filtered data */
                bmi0xx_write_register(BMI_ACC_CS, BMA2x2_INTR1_PAD_SELECT_ADDR,     0x00);                          /* disable all un-related interrupts */
                bmi0xx_write_register(BMI_ACC_CS, BMA2x2_INTR_DATA_SELECT_ADDR,     0x66);                          /* enable watermark interrupt */
                bmi0xx_write_register(BMI_ACC_CS, BMA2x2_INTR2_PAD_SELECT_ADDR,     0x00);                          /* disable all un-related interrupts */
                bmi0xx_write_register(BMI_ACC_CS, BMA2x2_SLOPE_DURN_ADDR,           0x01);
                bmi0xx_write_register(BMI_ACC_CS, BMA2x2_SLOPE_THRES_ADDR,          0x0A);
                bmi0xx_write_register(BMI_ACC_CS, BMA2x2_INTR_ENABLE0_ADDR,         0x00);
                bmi0xx_write_register(BMI_ACC_CS, BMA2x2_INTR_ENABLE1_ADDR,         0x60);                          /* 0x40: wm  0x20: fifo full */
                bmi0xx_write_register(BMI_ACC_CS, BMA2x2_INTR_ENABLE2_ADDR,         0x00); 
                bmi0xx_write_register(BMI_ACC_CS, BMA2x2_INTR_SLOW_NO_MOTION_ADDR,  0x00);
            }
            else if(rt_bmi.acc_chip_id == BMI088_ACC_CHIP_ID)
            {
                bmi0xx_write_register(BMI_ACC_CS, 0x53,             (1<<3) | (1<<1));    
                bmi0xx_write_register(BMI_ACC_CS, 0x54,             (1<<3) | (1<<1));  
                
                bmi0xx_write_register(BMI_ACC_CS, BMI08X_ACC_FIFO_CFG0,             0x03);                          /* FIFO mode */
                bmi0xx_write_register(BMI_ACC_CS, BMI08X_ACC_FIFO_CFG1,             (1<<6) | (1<<4) | (1<<3) | (1<<2));                          /* enable ACC FIFO, INT1 INT2 */
                bmi0xx_write_register(BMI_ACC_CS, BMI08X_INT1_INT2_MAP_DATA,        0x22);                          /* enable water mark*/
                bmi0xx_write_register(BMI_ACC_CS, BMI08X_FIFO_WTM0,                 rt_bmi.acc_fifo_read_size*7);     /* WM0 */
                bmi0xx_write_register(BMI_ACC_CS, BMI08X_FIFO_WTM1,                 0x00);                          /* WM1 */
                bmi0xx_write_register(BMI_ACC_CS, 0x7E, 0xB0); 
            }
        

            break;
        case RT_SENSOR_POS_GYR_WM_FIFO_INT:
            bmi0xx_write_register(BMI_GYR_CS, BMG160_INTR_MAP_ZERO_ADDR,    0x00);                              /* disable high_rate, any-morion interrupt */
            bmi0xx_write_register(BMI_GYR_CS, BMG160_INTR_MAP_ONE_ADDR,     0x04);                              /* enable fifo interrupt to INT3 */
            bmi0xx_write_register(BMI_GYR_CS, BMG160_INTR_MAP_TWO_ADDR,     0x00);                              /* disable high-rate any-motion interrupt to INT2 */
            bmi0xx_write_register(BMI_GYR_CS, BMG160_INTR_ENABLE1_ADDR,     0x01);                              /* set active level high for INT3 */
            bmi0xx_write_register(BMI_GYR_CS, BMG160_HIGH_BW_ADDR,          0x80);                              /* use un-filerted data */
            bmi0xx_write_register(BMI_GYR_CS, BMG160_INTR_4_ADDR,           0x80);                              /* enable fifo watermark interruppt */
            bmi0xx_write_register(BMI_GYR_CS, BMG160_FIFO_CGF1_ADDR,        0x8C);                              /* fifo use FIFO mode 8C:STREAM, 4C:FIFO*/
            bmi0xx_write_register(BMI_GYR_CS, BMG160_FIFO_CGF0_ADDR,        rt_bmi.gyr_fifo_read_size);         /* collet n sample trigger 1 interrupt */
            bmi0xx_write_register(BMI_GYR_CS, BMG160_INTR_ENABLE0_ADDR,     0x40);                              /* enable FIFO enable interrupt */
        
            /* enable acc sensor acc interrupt */
            rt_pin_mode(rt_bmi.acc_int_pin, PIN_MODE_INPUT);
            rt_pin_attach_irq(rt_bmi.acc_int_pin, PIN_IRQ_MODE_RISING, bmi0xx_acc_pin_int_handler, RT_NULL);
            rt_pin_irq_enable(rt_bmi.acc_int_pin, PIN_IRQ_ENABLE);

            /* enable raw sensor gyr interrupt */
            rt_pin_mode(rt_bmi.gyr_int_pin, PIN_MODE_INPUT);
            rt_pin_attach_irq(rt_bmi.gyr_int_pin, PIN_IRQ_MODE_RISING, bmi0xx_gyr_pin_int_handler, RT_NULL);
            rt_pin_irq_enable(rt_bmi.gyr_int_pin, PIN_IRQ_ENABLE);

        
            break;
        case RT_SENSOR_POS_ACC_BW:
            rt_bmi.acc_bw = *val;
            break;
        case RT_SENSOR_POS_ACC_RG:
            rt_bmi.acc_rg = *val;
            rt_bmi.acc_ratio = rt_bmi.acc_rg_tb[rt_bmi.acc_rg] / 32768.0F;
            if(rt_bmi.acc_chip_id == BMI055_ACC_CHIP_ID)
            {
                switch(rt_bmi.acc_rg)
                {
                    case 0:
                        bmi0xx_write_register(BMI_ACC_CS, BMA2x2_RANGE_SELECT_ADDR, 0x03);
                        break;
                    case 1:
                        bmi0xx_write_register(BMI_ACC_CS, BMA2x2_RANGE_SELECT_ADDR, 0x05);
                        break;
                    case 2:
                        bmi0xx_write_register(BMI_ACC_CS, BMA2x2_RANGE_SELECT_ADDR, 0x08);
                        break;
                    case 3:
                        bmi0xx_write_register(BMI_ACC_CS, BMA2x2_RANGE_SELECT_ADDR, 0x0C);
                        break;
                }
            }
            else
            {
                switch(rt_bmi.acc_rg)
                {
                    case 0:
                        bmi0xx_write_register(BMI_ACC_CS, BMI08X_ACC_RANGE, 0x00);
                        break;
                    case 1:
                        bmi0xx_write_register(BMI_ACC_CS, BMI08X_ACC_RANGE, 0x01);
                        break;
                    case 2:
                        bmi0xx_write_register(BMI_ACC_CS, BMI08X_ACC_RANGE, 0x02);
                        break;
                    case 3:
                        bmi0xx_write_register(BMI_ACC_CS, BMI08X_ACC_RANGE, 0x03);
                        break;
                }
            }
            break;
        case RT_SENSOR_POS_GYR_BW:
            rt_bmi.gyr_bw = *val;
            break;
        case RT_SENSOR_POS_GYR_RG:
            rt_bmi.gyr_rg = *val;
            rt_bmi.gyr_ratio = rt_bmi.gyr_rg_tb[rt_bmi.gyr_rg] / 32768.0F;
            break;
    }
    return size;
}



static rt_err_t  rt_bmi055_control(rt_device_t dev, int cmd, void *args)
{
    return RT_EOK;
}

int rt_hw_bmi0xx_init(const char *name, uint32_t acc_int_pin, uint32_t gyr_int_pin)
{
    struct rt_device *dev;
    
    dev                     = &rt_bmi.parent;
	dev->type               = RT_Device_Class_Miscellaneous;
	dev->rx_indicate        = RT_NULL;
	dev->tx_complete        = RT_NULL;
	dev->init               = rt_bmi0xx_init;
	dev->open               = rt_bmi055_open;
	dev->close              = RT_NULL;
	dev->read               = rt_bmi0xx_read;
	dev->write              = rt_bmi055_write;
	dev->control            = rt_bmi055_control;
	dev->user_data          = &rt_bmi;
 
    rt_bmi.acc_int_pin = acc_int_pin;
    rt_bmi.gyr_int_pin = gyr_int_pin;
    rt_bmi.gyr_fifo_read_size = 20;
    rt_bmi.acc_fifo_read_size = 5;
    
    rt_bmi.mtx = rt_mutex_create("bmi0xx", RT_IPC_FLAG_FIFO);
    
    rt_device_register(dev, name, RT_DEVICE_FLAG_RDWR);
    return RT_EOK;
}


/* gyr int handler */
static void bmi0xx_gyr_pin_int_handler(void *args)
{
    rt_bmi.parent.rx_indicate(&rt_bmi.parent, 1<<RT_SENSOR_POS_GYR_FIFO);
}

/* acc int handler */
static void bmi0xx_acc_pin_int_handler(void *args)
{
    rt_bmi.parent.rx_indicate(&rt_bmi.parent, 1<<RT_SENSOR_POS_ACC_FIFO);
}

