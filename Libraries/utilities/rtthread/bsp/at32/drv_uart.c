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
#include "dma.h"


#define AT32_UART_CNT        (4)

struct at32_uart_t
{
    USART_Type *UARTx;
    uint32_t setting_recv_len;      /* setting receive len */
    uint32_t last_recv_index;       /* last receive index */
    uint32_t tx_dma_ch;
    uint32_t rx_dma_ch;
    uint32_t rx_buf_addr;
    uint32_t instance;
};

struct at32_uart_t at32_uart[AT32_UART_CNT];
struct rt_serial_device serial[AT32_UART_CNT];

static void uart_setup_tx_dma(struct at32_uart_t *uart, uint8_t *buf, uint32_t len)
{
    DMA_ChlSetup_t dma;
    
    dma.chl = uart->tx_dma_ch;
    dma.dataWidth = DMA_DATAWIDTH_1B;
    dma.dir = DMA_DIR_MEM2PERIPH;
    dma.size = len;
    
    dma.MemAddr = (uint32_t)buf;
    dma.MemInc = 1;
    
    dma.PeriphAddr = (uint32_t)&uart->UARTx->DT;
    dma.PeriphInc = 0;
    
    DMA_SetupChl(HW_DMA1, &dma);
}


static void uart_setup_rx_dma(struct at32_uart_t *uart, uint8_t *buf, uint32_t len)
{
    DMA_ChlSetup_t dma;
    
    dma.chl = uart->rx_dma_ch;
    dma.dataWidth = DMA_DATAWIDTH_1B;
    dma.dir = DMA_DIR_PERIPH2MEM;
    dma.size = len;
    
    dma.MemAddr = (uint32_t)buf;
    dma.MemInc = 1;
    
    dma.PeriphAddr = (uint32_t)&uart->UARTx->DT;
    dma.PeriphInc = 0;
    
    DMA_SetupChl(HW_DMA1, &dma);
}


static rt_err_t at32_configure(struct rt_serial_device *serial, struct serial_configure *cfg)
{
    struct at32_uart_t *uart = (struct at32_uart_t *)serial->parent.user_data;

    RT_ASSERT(serial != RT_NULL);
    RT_ASSERT(cfg != RT_NULL);

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
  
    DMA_Init(HW_DMA1);
    DMA_SetChlIntMode(HW_DMA1, uart->tx_dma_ch, 1);
    UART_Init(uart->instance, cfg->baud_rate);
    UART_EnableTxDMA(uart->instance, 1);
    uart_setup_tx_dma(uart, NULL, 0);
    
    return RT_EOK;
}

static rt_err_t at32_control(struct rt_serial_device *serial, int cmd, void *arg)
{
    struct at32_uart_t *uart = (struct at32_uart_t *)serial->parent.user_data;
    uint32_t arg_val;
    
    RT_ASSERT(serial != RT_NULL);

    switch (cmd)
    {
        case RT_DEVICE_CTRL_CLR_INT:
            UART_SetIntMode(uart->instance, kUART_IntRx, 0);
            break;
        case RT_DEVICE_CTRL_SET_INT:
            UART_SetIntMode(uart->instance, kUART_IntRx, 1);
            break;
        case RT_DEVICE_CTRL_CONFIG:
            arg_val = (uint32_t)arg;
            
            if(arg_val == RT_DEVICE_FLAG_DMA_RX)
            {
                struct rt_serial_rx_fifo* rx_fifo = (struct rt_serial_rx_fifo*)serial->serial_rx;
                uart->rx_buf_addr = (uint32_t)rx_fifo->buffer;
                
                uart_setup_rx_dma(uart, (void*)uart->rx_buf_addr, serial->config.bufsz);
                uart->setting_recv_len = serial->config.bufsz;
                
                UART_EnableRxDMA(uart->instance, 1);
                DMA_SetChlIntMode(HW_DMA1, uart->rx_dma_ch, 1);
                UART_SetIntMode(uart->instance, kUART_IdleRx, 1);
                DMA_Enable(HW_DMA1, uart->rx_dma_ch, 1);
            }
            break;
        default:
            break;
  }

  return RT_EOK;
}

static int at32_putc(struct rt_serial_device *serial, char c)
{
    RT_ASSERT(serial != RT_NULL);
    struct at32_uart_t *uart =  (struct at32_uart_t *)serial->parent.user_data;

    #if (APP_PN == 316)
    rt_pin_write(30, 1);
    #endif

    UART_PutChar(uart->instance, c);
    return 1;
}

static int at32_getc(struct rt_serial_device *serial)
{
    RT_ASSERT(serial != RT_NULL);
    struct at32_uart_t *uart = (struct at32_uart_t *)serial->parent.user_data;

    if(uart->UARTx->STS & USART_STS_RDNE)
    {
        return uart->UARTx->DT;
    }
    return -1;
}

static rt_size_t at32_dma_transmit(struct rt_serial_device *serial, rt_uint8_t *buf, rt_size_t size, int direction)
{
    struct at32_uart_t *uart = (struct at32_uart_t *)serial->parent.user_data;
    
    if(direction == RT_SERIAL_DMA_TX)
    {
        
        #if (APP_PN == 316)
        rt_pin_write(30, 1);
        #endif
            
        DMA_Enable(HW_DMA1, uart->tx_dma_ch, 0);
        DMA_SetChlMemAddr(HW_DMA1, uart->tx_dma_ch, (uint32_t)buf);
        DMA_SetChlLen(HW_DMA1, uart->tx_dma_ch, size);
        DMA_Enable(HW_DMA1, uart->tx_dma_ch, 1);
    }
    
    if(direction == RT_SERIAL_DMA_RX)
    {
        
    }

    return size;
}

static const struct rt_uart_ops at32_uart_ops =
{
    at32_configure,
    at32_control,
    at32_putc,
    at32_getc,
    at32_dma_transmit,
};


int rt_hw_uart_init(const char *name, int baud)
{
    uint32_t instance;
    struct serial_configure config = RT_SERIAL_CONFIG_DEFAULT;
    config.bufsz = 256;
    config.baud_rate = baud;
    
    sscanf(name, "uart%d", &instance);
    
    at32_uart[instance].instance = instance;
    serial[instance].ops = &at32_uart_ops;
    serial[instance].config = config;
    
    switch(instance)
    {
        case HW_UART1:
            at32_uart[instance].tx_dma_ch = HW_DMA_CH4;
            at32_uart[instance].rx_dma_ch = HW_DMA_CH5;
            at32_uart[instance].UARTx = USART1;
            break;
        case HW_UART2:
            at32_uart[instance].tx_dma_ch = HW_DMA_CH7;
            at32_uart[instance].rx_dma_ch = HW_DMA_CH6;
            at32_uart[instance].UARTx = USART2;
            break;
        case HW_UART3:
            at32_uart[instance].tx_dma_ch = HW_DMA_CH2;
            at32_uart[instance].rx_dma_ch = HW_DMA_CH3;
            at32_uart[instance].UARTx = USART3;
            break;
    }

    /* register device */
    rt_hw_serial_register(&serial[instance], name, RT_DEVICE_FLAG_RDWR  | RT_DEVICE_FLAG_DMA_TX | RT_DEVICE_FLAG_DMA_RX | RT_DEVICE_FLAG_INT_RX, &at32_uart[instance]);
    return 0;
}



void USART1_IRQHandler(void)
{
    volatile uint8_t dummy, sts;
    
    rt_interrupt_enter();
    sts = USART1->STS;
    
    if(sts & USART_STS_RDNE)
    {
        rt_hw_serial_isr(&serial[1], RT_SERIAL_EVENT_RX_IND);
    }
    
    /* overrun ,clear OVERRUN bit */
    if(sts & USART_STS_ORERR)
    {
        dummy = USART1->DT;
    }
    
    if(sts & USART_STS_TRAC)
    {
        /* disable Tx done */
        USART1->CTRL1 &= ~USART_CTRL1_TRACIEN;
        
        #if (APP_PN == 316)
        rt_pin_write(30, 0);
        #endif
        
    }
    
    if(sts & USART_STS_IDLEF)
    {
        dummy = USART1->DT;
        uint32_t recv_total_index, recv_len; 

        recv_total_index = at32_uart[1].setting_recv_len - DMA_GetTransferCnt(HW_DMA1, at32_uart[1].rx_dma_ch);
        recv_len = recv_total_index - at32_uart[1].last_recv_index;
        at32_uart[1].last_recv_index = recv_total_index;
        
        rt_hw_serial_isr(&serial[1], RT_SERIAL_EVENT_RX_DMADONE | (recv_len << 8));
    }
    
    rt_interrupt_leave();
}

void USART2_IRQHandler(void)
{
    volatile uint8_t dummy, sts;
    
    rt_interrupt_enter();
    
    sts = USART2->STS;
    
    if(sts & USART_STS_RDNE)
    {
        rt_hw_serial_isr(&serial[2], RT_SERIAL_EVENT_RX_IND);
    }
    
    /* overrun ,clear OVERRUN bit */
    if(sts & USART_STS_ORERR)
    {
        dummy = USART2->DT;
    }
    
    if(sts & USART_STS_TRAC)
    {
        /* disable Tx done */
        USART2->CTRL1 &= ~USART_CTRL1_TRACIEN;
        
    }
    
    if(sts & USART_STS_IDLEF)
    {
        dummy = USART2->DT;
        uint32_t recv_total_index, recv_len; 

        recv_total_index = at32_uart[2].setting_recv_len - DMA_GetTransferCnt(HW_DMA1, at32_uart[2].rx_dma_ch);
        recv_len = recv_total_index - at32_uart[2].last_recv_index;
        at32_uart[2].last_recv_index = recv_total_index;
        
        rt_hw_serial_isr(&serial[2], RT_SERIAL_EVENT_RX_DMADONE | (recv_len << 8));
    }
    
    rt_interrupt_leave();
}

void USART3_IRQHandler(void)
{
    rt_interrupt_enter();
    
    if(USART3->STS & USART_STS_RDNE)
    {
        rt_hw_serial_isr(&serial[3], RT_SERIAL_EVENT_RX_IND);
    }

    rt_interrupt_leave();
}


/* uart1 dma tx interrupt */
void DMA1_Channel4_IRQHandler(void)
{
    rt_interrupt_enter();
    DMA1->ICLR = DMA_ICLR_CTCIF4 | DMA_ICLR_CGIF4 | DMA_ICLR_CHTIF4 | DMA_ICLR_CERRIF4;

    /* enable Tx done, for dma interrupt is too early for last byte to be transmitted */
    USART1->CTRL1 |= USART_CTRL1_TRACIEN;
    
    rt_hw_serial_isr(&serial[1], RT_SERIAL_EVENT_TX_DMADONE);
    rt_interrupt_leave();
}

/* uart1 dma tx interrupt */
void DMA1_Channel5_IRQHandler(void)
{
    rt_size_t recv_len;
    
    rt_interrupt_enter();
    DMA1->ICLR = DMA_ICLR_CTCIF5 | DMA_ICLR_CGIF5 | DMA_ICLR_CHTIF5 | DMA_ICLR_CERRIF5;
    rt_interrupt_leave();
    
    recv_len = at32_uart[1].setting_recv_len - at32_uart[1].last_recv_index;
    
    /* reset last recv index */
    at32_uart[1].last_recv_index = 0;
 
    DMA_Enable(HW_DMA1, at32_uart[1].rx_dma_ch, 0);
    DMA_SetChlMemAddr(HW_DMA1, at32_uart[1].rx_dma_ch, at32_uart[1].rx_buf_addr);
    DMA_SetChlLen(HW_DMA1, at32_uart[1].rx_dma_ch, at32_uart[1].setting_recv_len);
    DMA_Enable(HW_DMA1, at32_uart[1].rx_dma_ch, 1);
    
    rt_hw_serial_isr(&serial[1], RT_SERIAL_EVENT_RX_DMADONE | (recv_len << 8));
}


/* uart2 dma tx interrupt */
void DMA1_Channel7_IRQHandler(void)
{
    rt_interrupt_enter();
    DMA1->ICLR = DMA_ICLR_CTCIF7 | DMA_ICLR_CGIF7 | DMA_ICLR_CHTIF7 | DMA_ICLR_CERRIF7;

    /* enable Tx done, for dma interrupt is too early for last byte to be transmitted */
    USART2->CTRL1 |= USART_CTRL1_TRACIEN;
    
    rt_hw_serial_isr(&serial[2], RT_SERIAL_EVENT_TX_DMADONE);
    rt_interrupt_leave();
}


/* uart2 dma tx interrupt */
void DMA1_Channel6_IRQHandler(void)
{
    rt_size_t recv_len;
    
    rt_interrupt_enter();
    DMA1->ICLR = DMA_ICLR_CTCIF6 | DMA_ICLR_CGIF6 | DMA_ICLR_CHTIF6 | DMA_ICLR_CERRIF6;
    rt_interrupt_leave();
    
    recv_len = at32_uart[2].setting_recv_len - at32_uart[2].last_recv_index;
    
    /* reset last recv index */
    at32_uart[2].last_recv_index = 0;
 
    DMA_Enable(HW_DMA1, at32_uart[2].rx_dma_ch, 0);
    DMA_SetChlMemAddr(HW_DMA1, at32_uart[2].rx_dma_ch, at32_uart[2].rx_buf_addr);
    DMA_SetChlLen(HW_DMA1, at32_uart[2].rx_dma_ch, at32_uart[2].setting_recv_len);
    DMA_Enable(HW_DMA1, at32_uart[2].rx_dma_ch, 1);
    
    rt_hw_serial_isr(&serial[2], RT_SERIAL_EVENT_RX_DMADONE | (recv_len << 8));
}

