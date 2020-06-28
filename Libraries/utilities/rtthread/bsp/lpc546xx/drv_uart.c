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
#include <string.h>
#include "common.h"
#include "uart.h"
#include "dma.h"
#include <rtdevice.h>

#define HW_LPC546XX_UART_CNT        (10)

/* LP546xx uart driver */
struct lpc546xx_uart
{
    uint8_t instance;
    USART_Type *USARTx;
    DMA_ChlSetup_t tx_setup;
};

struct lpc546xx_uart uart[HW_LPC546XX_UART_CNT];
struct rt_serial_device serial[HW_LPC546XX_UART_CNT];


static rt_err_t lpc546xx_configure(struct rt_serial_device *serial, struct serial_configure *cfg)
{
    struct lpc546xx_uart *uart;

    RT_ASSERT(serial != RT_NULL);
    RT_ASSERT(cfg != RT_NULL);

    uart = (struct lpc546xx_uart *)serial->parent.user_data;
    
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
    
    if(serial->parent.flag & RT_DEVICE_FLAG_DMA_TX)
    {
        uart->tx_setup.isPeriph = true;
        uart->tx_setup.dataWidth = 1;
        
        uart->tx_setup.sAddrInc = 1;
        uart->tx_setup.dAddrInc = 0;
        
        switch(uart->instance)
        {
            case 4:
                uart->tx_setup.chl = DMAREQ_FLEXCOMM4_TX;
                uart->tx_setup.dAddr = (uint32_t)(&(USART4->FIFOWR));
                break;
            case 0:
                uart->tx_setup.chl = DMAREQ_FLEXCOMM0_TX;
                uart->tx_setup.dAddr = (uint32_t)(&(USART0->FIFOWR));
                break;
            default:
                printf("lpc546xx_configure error instance%d\r\n", uart->instance);
                break;
        }
        
        DMA_SetupChl(&uart->tx_setup);
        DMA_SetChlIntMode(uart->tx_setup.chl, true);
        UART_SetDMAMode(uart->instance, kUART_DMATx, true);
    }
    return RT_EOK;
}

static rt_err_t lpc546xx_control(struct rt_serial_device *serial, int cmd, void *arg)
{
    struct lpc546xx_uart *uart;

    RT_ASSERT(serial != RT_NULL);
    uart = (struct lpc546xx_uart *)serial->parent.user_data;

    switch (cmd)
    {
        case RT_DEVICE_CTRL_CLR_INT:
            UART_SetIntMode(uart->instance, kUART_IntRx, false);
            break;
        case RT_DEVICE_CTRL_SET_INT:;
            UART_SetIntMode(uart->instance, kUART_IntRx, true);
            break;
  }

  return RT_EOK;
}

static int lpc546xx_putc(struct rt_serial_device *serial, char c)
{
    struct lpc546xx_uart *uart;

    RT_ASSERT(serial != RT_NULL);
    uart = (struct lpc546xx_uart *)serial->parent.user_data;
    
    UART_PutChar(uart->instance, c);
    return 1;
}

static int lpc546xx_getc(struct rt_serial_device *serial)
{
    uint8_t ch;
    struct lpc546xx_uart *uart;

    RT_ASSERT(serial != RT_NULL);
    uart = (struct lpc546xx_uart *)serial->parent.user_data;

    if(UART_GetChar(uart->instance, &ch) == RT_EOK)
    {
        return ch;
    }
    return -1;
}

static rt_size_t lpc546xx_dma_transmit(struct rt_serial_device *serial, rt_uint8_t *buf, rt_size_t size, int direction)
{
    struct lpc546xx_uart *uart;

    RT_ASSERT(serial != RT_NULL);
    uart = (struct lpc546xx_uart *)serial->parent.user_data;
    
    if(direction == RT_SERIAL_DMA_TX)
    {
        uart->tx_setup.transferCnt = size;
        uart->tx_setup.sAddr = (uint32_t)buf;
        
        DMA_SetupChl(&uart->tx_setup);
        DMA_SWTrigger(uart->tx_setup.chl);
    }
    return size;
}
    
static const struct rt_uart_ops lpc546xx_uart_ops =
{
    lpc546xx_configure,
    lpc546xx_control,
    lpc546xx_putc,
    lpc546xx_getc,
    lpc546xx_dma_transmit,
};

static void uart_isr(struct rt_serial_device *serial)
{
    struct lpc546xx_uart *uart = (struct lpc546xx_uart *) serial->parent.user_data;

    RT_ASSERT(uart != RT_NULL);

    if(uart->USARTx->FIFOSTAT & USART_FIFOSTAT_RXNOTEMPTY_MASK)
    {
        rt_hw_serial_isr(serial, RT_SERIAL_EVENT_RX_IND);
    }
    
}

void FLEXCOMM0_IRQHandler(void)
{
    rt_interrupt_enter();
    uart_isr(&serial[0]);
    rt_interrupt_leave();
}

void FLEXCOMM1_IRQHandler(void)
{
    rt_interrupt_enter();
    uart_isr(&serial[1]);
    rt_interrupt_leave();
}

void FLEXCOMM2_IRQHandler(void)
{
    rt_interrupt_enter();
    uart_isr(&serial[2]);
    rt_interrupt_leave();
}

void FLEXCOMM3_IRQHandler(void)
{
    rt_interrupt_enter();
    uart_isr(&serial[3]);
    rt_interrupt_leave();
}

void FLEXCOMM4_IRQHandler(void)
{
    rt_interrupt_enter();
    uart_isr(&serial[4]);
    rt_interrupt_leave();
}

void FLEXCOMM5_IRQHandler(void)
{
    rt_interrupt_enter();
    uart_isr(&serial[5]);
    rt_interrupt_leave();
}


//void DMA0_IRQHandler(void)
//{
//    rt_interrupt_enter();
//    
//    int i;
//    static volatile uint32_t INTA;
//    
//    INTA = DMA0->COMMON[0].INTA;
//    DMA0->COMMON[0].INTA = DMA0->COMMON[0].INTA;
//    
//    if(INTA & (1 << DMAREQ_FLEXCOMM4_TX))
//    {
//        rt_hw_serial_isr(&serial[4], RT_SERIAL_EVENT_TX_DMADONE);
//    }
//    
//    rt_interrupt_leave();
//}

int rt_hw_uart_init(const char *name)
{
    uint32_t instance;
    struct serial_configure config = RT_SERIAL_CONFIG_DEFAULT;
    static USART_Type* const uart_instance_table[] = USART_BASE_PTRS;
    
    sscanf(name, "uart%d", &instance);
    
    uart[instance].USARTx = uart_instance_table[instance];
    uart[instance].instance = instance;
    serial[instance].ops    = &lpc546xx_uart_ops;
    serial[instance].config = config;

    /* register  device */
    rt_hw_serial_register(&serial[instance], name, RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX | RT_DEVICE_FLAG_DMA_TX, &uart[instance]);
                         

    return 0;
}
