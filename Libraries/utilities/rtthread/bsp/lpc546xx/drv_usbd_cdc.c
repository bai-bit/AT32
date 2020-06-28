
#include <rtthread.h>

#include "common.h"
#include "usbd.h"
#include "usbd_cdc.h"

#define CDC_NAME                    "usbd_cdc"
#define CDC_BUF_SIZE                (64)

struct bsp_drv_cdc_device
{
    struct rt_device                rtdev;
    rt_mutex_t                      lock;
    rt_sem_t                        tx_sem;
    rt_mq_t                         rx_mq;
    uint8_t                         is_line_active;
};

/* device need to implment host's config */
static uint32_t set_line_coding(struct ucdc_line_coding *line_coding)
{
   // USBD_TRACE("set line coding:%d\r\n", line_coding->dwDTERate);
    return 0;
}

static uint32_t cdc_send_notify(void)
{
    struct bsp_drv_cdc_device *cdc_dev = (struct bsp_drv_cdc_device *)rt_device_find(CDC_NAME);
    rt_sem_release(cdc_dev->tx_sem);
    return 0;
}

static uint32_t cdc_recv_handler(uint8_t *buf, uint32_t len)
{
    struct bsp_drv_cdc_device *cdc_dev = (struct bsp_drv_cdc_device *)rt_device_find(CDC_NAME);
    if(cdc_dev)
    {
        for(int i=0; i<len; i++)
        {
            rt_mq_send(cdc_dev->rx_mq, &buf[i], 1);
        }
        
        if(cdc_dev->rtdev.rx_indicate)
        {
            cdc_dev->rtdev.rx_indicate((rt_device_t)cdc_dev, len); 
        }
    }
    return 0;
}


    
/* host need device's line_coding config */
static uint32_t get_line_coding(struct ucdc_line_coding *line_coding)
{
    line_coding->dwDTERate = 115200;
    line_coding->bCharFormat = 2;
   // USBD_TRACE("get_line_coding, baud:%d\r\n", line_coding->dwDTERate);
    return 0;
}

static uint32_t set_control_line_serial_state(uint8_t val)
{
    struct bsp_drv_cdc_device *cdc_dev = (struct bsp_drv_cdc_device *)rt_device_find(CDC_NAME);
    
    /* 0:off, 1 on */
    cdc_dev->is_line_active = val;
    return 0;
}

static struct usbd_cdc_callback_t cdc_cb = 
{
    get_line_coding,
    set_line_coding,
    set_control_line_serial_state,
    cdc_recv_handler,
    cdc_send_notify,
};

static void _lock(struct bsp_drv_cdc_device * dev)
{
    rt_mutex_take(dev->lock, RT_WAITING_FOREVER);
}

static void _unlock(struct bsp_drv_cdc_device * dev)
{
    rt_mutex_release(dev->lock);
}


static rt_err_t cdc_init (rt_device_t dev)
{    
    return RT_EOK;
}

static rt_size_t cdc_read(rt_device_t dev, rt_off_t pos, void* buffer, rt_size_t size)
{
    int i;
    struct bsp_drv_cdc_device * cdc_dev;
    cdc_dev = (struct bsp_drv_cdc_device*)dev;
    
    _lock(cdc_dev);
    
    cdc_dev = (struct bsp_drv_cdc_device*)dev;
    
    uint8_t *p = buffer;
    for(i=0; i<size; i++)
    {
        if(rt_mq_recv(cdc_dev->rx_mq, &p[i], 1, 0) != RT_EOK)
        {
            _unlock(cdc_dev);
            return i;
        }
    }
    
    _unlock(cdc_dev);
    return size;
}

static rt_size_t cdc_write(rt_device_t dev, rt_off_t pos, const void* buffer, rt_size_t size)
{
    struct bsp_drv_cdc_device *cdc_dev = (struct bsp_drv_cdc_device*)dev;
    
    _lock(cdc_dev);
    
    uint8_t *p = (uint8_t*)buffer;
    
    if(cdc_dev->is_line_active == 1)
    {
        while(size--)
        {
            if (*p == '\n' && (dev->open_flag & RT_DEVICE_FLAG_STREAM))
            {
                uint8_t r = '\r';
                if(rt_sem_take(cdc_dev->tx_sem, 1) == RT_EOK)
                {
                    usbd_cdc_send(&r, 1);
                }
            }
        
            if(rt_sem_take(cdc_dev->tx_sem, 1) == RT_EOK)
            {
                usbd_cdc_send(p, 1);
            }
            p++;
        }
    }


    _unlock(cdc_dev);
    
    return size;
}

static rt_err_t cdc_open(rt_device_t dev, rt_uint16_t oflag)
{
    return RT_EOK;
}

static rt_err_t cdc_control(rt_device_t dev, rt_uint8_t cmd, void *args)
{
    return RT_EOK; 
}

int rt_hw_usbd_cdc_init(const char *name)
{
    struct bsp_drv_cdc_device *cdc_dev;
    
    /* already registiered */
    if(rt_device_find(name))
    {
        return -RT_EIO;
    }
    
    cdc_dev = rt_malloc(sizeof(struct bsp_drv_cdc_device));
	cdc_dev->rtdev.type 		= RT_Device_Class_Char;
	cdc_dev->rtdev.rx_indicate  = RT_NULL;
	cdc_dev->rtdev.tx_complete  = RT_NULL;
	cdc_dev->rtdev.init 		= cdc_init;
	cdc_dev->rtdev.open         = cdc_open;
	cdc_dev->rtdev.close		= RT_NULL;
	cdc_dev->rtdev.read 		= cdc_read;
	cdc_dev->rtdev.write        = cdc_write;
	cdc_dev->rtdev.control      = cdc_control;
	cdc_dev->rtdev.user_data	= RT_NULL;
    
    cdc_dev->lock = rt_mutex_create(name, RT_IPC_FLAG_FIFO);
    cdc_dev->tx_sem = rt_sem_create(name, 1, RT_IPC_FLAG_FIFO);
    cdc_dev->rx_mq = rt_mq_create(name, 1, CDC_BUF_SIZE, RT_IPC_FLAG_FIFO);
    
    usbd_cdc_set_cb(&cdc_cb);
    
    rt_device_register(&cdc_dev->rtdev, name, RT_DEVICE_FLAG_RDWR);
    return RT_EOK;
}


INIT_BOARD_EXPORT(rt_hw_uart_init);
