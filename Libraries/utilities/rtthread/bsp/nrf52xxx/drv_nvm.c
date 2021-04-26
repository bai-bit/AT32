#include <rtthread.h>
#include <stdio.h>

#include "common.h"
#include "flash.h"

#define NVM_PAGE_SIZE               (4096)
#define NVM_START_ADDR              ((32+100)*1024)

struct nvm_device
{
    struct rt_device                rtdev;
    struct rt_mutex                 lock;
    uint32_t                        nvm_start_addr;
};

static void _lock(struct nvm_device * dev)
{
    rt_mutex_take(&dev->lock, RT_WAITING_FOREVER);
}

static void _unlock(struct nvm_device * dev)
{
    rt_mutex_release(&dev->lock);
}

static rt_err_t rt_nvm_init (rt_device_t dev)
{    
   // struct nvm_device * nvm_dev = (struct nvm_device*)dev;
    return RT_EOK;
}

static rt_size_t rt_nvm_read(rt_device_t dev, rt_off_t pos, void* buffer, rt_size_t size)
{
    int len;
    _lock((struct nvm_device *)dev);
    
    rt_memcpy(buffer, (uint8_t*)(pos*NVM_PAGE_SIZE + NVM_START_ADDR), size*NVM_PAGE_SIZE);
    
    _unlock((struct nvm_device *)dev);
    return len;
}

static rt_size_t rt_nvm_write(rt_device_t dev, rt_off_t pos, const void* buffer, rt_size_t size)
{
//    struct nvm_device * nvm_dev;
    
    _lock((struct nvm_device *)dev);
    

    _unlock((struct nvm_device *)dev);
    
    return size;
}

int rt_hw_nvm_init(const char *name)
{
    struct nvm_device *dev;
    
    if(rt_device_find(name))
    {
        return -RT_EIO;
    }
    
    dev = rt_malloc(sizeof(struct nvm_device));
	dev->rtdev.type 		= RT_Device_Class_Block;
	dev->rtdev.rx_indicate  = RT_NULL;
	dev->rtdev.tx_complete  = RT_NULL;
	dev->rtdev.init 		= rt_nvm_init;
	dev->rtdev.open         = RT_NULL;
	dev->rtdev.close		= RT_NULL;
	dev->rtdev.read 		= rt_nvm_read;
	dev->rtdev.write        = rt_nvm_write;
//	dev->rtdev.control      = rt_nvm_control;
	dev->rtdev.user_data	= RT_NULL;
    
    /* initialize mutex */
    if (rt_mutex_init(&dev->lock, name, RT_IPC_FLAG_FIFO) != RT_EOK)
    {
        return -RT_ENOSYS;
    }
    
    rt_device_register(&dev->rtdev, name, RT_DEVICE_FLAG_RDWR);
    return RT_EOK;
}
