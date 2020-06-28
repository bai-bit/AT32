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
#include <stdio.h>
#include <rtdevice.h>
#include "common.h"
#include "uart.h"

#define NRF_UART_CNT        (1)

struct nrf_uart_t
{
    NRF_UARTE_Type  *UARTx;
    uint8_t instance;
};

static NRF_UARTE_Type* const uart_instance_table[] = {NRF_UARTE0};
struct nrf_uart_t nrf_uart[NRF_UART_CNT];
struct rt_serial_device serial[NRF_UART_CNT];

static rt_err_t nrf_configure(struct rt_serial_device *serial, struct serial_configure *cfg)
{
    struct nrf_uart_t *uart;

    RT_ASSERT(serial != RT_NULL);
    RT_ASSERT(cfg != RT_NULL);

    uart = (struct nrf_uart_t *)serial->parent.user_data;
    switch (cfg->data_bits)
    {
        case DATA_BITS_8:
            break;
    }
    
    switch (cfg->stop_bits)
    {
        case STOP_BITS_1:
            break;
    }
      
    switch (cfg->parity)
    {
        case PARITY_NONE:
            break;
    }
  
    UART_Init(uart->instance, cfg->baud_rate);
    return RT_EOK;
}

static rt_err_t nrf_control(struct rt_serial_device *serial, int cmd, void *arg)
{
    uint32_t arg_val;
    struct nrf_uart_t *uart;

    RT_ASSERT(serial != RT_NULL);
    uart = (struct nrf_uart_t *)serial->parent.user_data;
    switch (cmd)
    {
        case RT_DEVICE_CTRL_CLR_INT:
            UART_SetIntMode(uart->instance, kUART_IntRx, false);
            break;
        case RT_DEVICE_CTRL_SET_INT:
           UART_SetIntMode(uart->instance, kUART_IntRx, true);
            break;
        case RT_DEVICE_CTRL_CONFIG:
            arg_val = (uint32_t)arg;
            if(arg_val == RT_DEVICE_FLAG_DMA_RX)
            {
                
            }
            break;
        default:
            break;
  }

  return RT_EOK;
}

static int nrf_putc(struct rt_serial_device *serial, char c)
{
    UART_PutChar(HW_UART0, c);
    return 1;
}

static int nrf_getc(struct rt_serial_device *serial)
{
    uint8_t c;
    if(UART_GetChar(HW_UART0, &c) == CH_OK)
    {
        return c;
    }
    
    return -1;
}

static rt_size_t nrf_dma_transmit(struct rt_serial_device *serial, rt_uint8_t *buf, rt_size_t size, int direction)
{
    if(direction == RT_SERIAL_DMA_TX)
    {
        UART_SetIntMode(HW_UART0, kUART_IntTx, true);
        UART_StartSend(HW_UART0, buf, size);
    }
    return size;
}

static const struct rt_uart_ops nrf_uart_ops =
{
    nrf_configure,
    nrf_control,
    nrf_putc,
    nrf_getc,
    nrf_dma_transmit,
};


int rt_hw_uart_init(const char *name, uint32_t baud)
{
    uint32_t instance;
    struct serial_configure config = RT_SERIAL_CONFIG_DEFAULT;
    config.bufsz = 32;
    config.baud_rate = baud;
    sscanf(name, "uart%d", &instance);
    
    if(rt_device_find(name) != RT_NULL)
    {
        return RT_EFULL;
    }
    
    nrf_uart[instance].instance = instance;
    nrf_uart[instance].UARTx = uart_instance_table[instance];
    serial[instance].ops    = &nrf_uart_ops;
    serial[instance].config = config;

    /* register device */
    rt_hw_serial_register(&serial[instance], name, RT_DEVICE_FLAG_RDWR  | RT_DEVICE_FLAG_DMA_TX | RT_DEVICE_FLAG_INT_RX, &nrf_uart[instance]);
    return 0;
}


void UARTE0_UART0_IRQHandler(void)
{
    /* for nRF52832, 64Mhz is not good for 921600 if add rt_interrupt_enter() */
 //   rt_interrupt_enter();
    
    if(NRF_UARTE0->EVENTS_ENDRX && (NRF_UARTE0->INTENSET & UARTE_INTENSET_ENDRX_Msk))
    {
        rt_hw_serial_isr(&serial[HW_UART0], RT_SERIAL_EVENT_RX_IND);
    }
    
    if(NRF_UARTE0->EVENTS_ENDTX && (NRF_UARTE0->INTENSET & UARTE_INTENSET_ENDTX_Msk))
    {
        NRF_UARTE0->EVENTS_ENDTX = 0;
        UART_SetIntMode(HW_UART0, kUART_IntTx, false);
        rt_hw_serial_isr(&serial[HW_UART0], RT_SERIAL_EVENT_TX_DMADONE);
    }
    
   // rt_interrupt_leave();
}


