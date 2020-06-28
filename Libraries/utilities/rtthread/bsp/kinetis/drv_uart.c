/*
 * File      : serial.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006 - 2012, RT-Thread Development Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Change Logs:
 * Date           Author       Notes
 * 2006-03-13     bernard      first version
 * 2012-05-15     lgnq         modified according bernard's implementation.
 * 2012-05-28     bernard      code cleanup
 * 2012-11-23     bernard      fix compiler warning.
 * 2013-02-20     bernard      use RT_SERIAL_RB_BUFSZ to define
 *                             the size of ring buffer.
 */

#include <rtthread.h>

#include "common.h"
#include "uart.h"

#define UART_BUF_SIZE               (128)

struct uart_device
{
    struct rt_device                rtdev;
    char                            rx_buf[UART_BUF_SIZE];
    uint32_t                        rx_len;
    uint32_t                        hw_instance;
    void (*hw_isr)(uint16_t data);
};


void UART1_RX_TX_IRQHandler(void)
{
    uint8_t ch;
    rt_interrupt_enter();
    if(UART_GetChar(HW_UART0, &ch) == CH_OK)
    {
        
    }
    struct uart_device *dev;

    dev = (struct uart_device *)rt_device_find("uart1");
    
    if(dev && dev->rx_len < UART_BUF_SIZE)
    {
        dev->rx_len++;
        dev->rx_buf[(dev->rx_len)-1] = ch;
        if(dev->rtdev.rx_indicate)
        {
            dev->rtdev.rx_indicate((rt_device_t)dev, dev->rx_len); 
        }
    }
    
    rt_interrupt_leave();
}


static rt_err_t rt_uart_init (rt_device_t dev)
{    
    UART_Init(UART0_RX_PD06_TX_PD07, 115200);
    UART_EnableTxFIFO(HW_UART0, true);
    UART_SetIntMode(HW_UART0, kUART_IntRx, true);
    return RT_EOK;
}

static rt_size_t rt_uart_read(rt_device_t dev, rt_off_t pos, void* buffer, rt_size_t size)
{
    int len;
    struct uart_device * uart_dev;
    uart_dev = (struct uart_device*)dev;
    len = uart_dev->rx_len;
    rt_memcpy(buffer, uart_dev->rx_buf, len);
    uart_dev->rx_len = 0;
    return len;
}

static rt_size_t rt_uart_write(rt_device_t dev, rt_off_t pos, const void* buffer, rt_size_t size)
{
    volatile uint8_t *p;
    struct uart_device * uart_dev;
    
    
    uart_dev = (struct uart_device*)dev;
    p = (uint8_t*)buffer;
    
    while(size--)
    {
        /*
         * to be polite with serial console add a line feed
         * to the carriage return character
         */
        if (*p == '\n' && (dev->open_flag & RT_DEVICE_FLAG_STREAM))
        {
            UART_PutChar(uart_dev->hw_instance, '\r');
        }
        UART_PutChar(uart_dev->hw_instance, *p++);
    }
    
    return size;
}

static rt_err_t rt_uart_control(rt_device_t dev, int cmd, void *args)
{
    return RT_EOK; 
}

int rt_hw_uart_init(const char *name)
{
    struct uart_device *dev;
    uint32_t instance;
    sscanf(name, "uart%d", &instance);
    
    if(rt_device_find(name))
    {
        return -RT_EIO;
    }
    
    dev = rt_malloc(sizeof(struct uart_device));
	dev->rtdev.type 		= RT_Device_Class_Char;
	dev->rtdev.rx_indicate  = RT_NULL;
	dev->rtdev.tx_complete  = RT_NULL;
	dev->rtdev.init 		= rt_uart_init;
	dev->rtdev.open         = RT_NULL;
	dev->rtdev.close		= RT_NULL;
	dev->rtdev.read 		= rt_uart_read;
	dev->rtdev.write        = rt_uart_write;
	dev->rtdev.control      = rt_uart_control;
	dev->rtdev.user_data	= RT_NULL;
    dev->hw_instance = instance;
    dev->rx_len = 0;
    
    
    rt_device_register(&dev->rtdev, name, RT_DEVICE_FLAG_RDWR);
    return RT_EOK;
}


INIT_BOARD_EXPORT(rt_hw_uart_init);
