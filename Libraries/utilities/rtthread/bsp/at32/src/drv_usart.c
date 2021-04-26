/*
 * Copyright (c) 2006-2020, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-01-06     shelton      first version
 */

//#include <board.h>
#include <string.h>
#include "drv_usart.h"
#include "at32f4xx.h"
#include "uart.h"

#ifdef RT_USING_SERIAL
#if !defined(BSP_USING_UART1) && !defined(BSP_USING_UART2) && \
    !defined(BSP_USING_UART3)
    #error "Please define at least one BSP_USING_UARTx"
    /* this driver can be disabled at menuconfig ¡ú RT-Thread Components ¡ú Device Drivers */
#endif

struct at32_usart {
    char *name;
    USART_Type* usartx;
    IRQn_Type irqn;
    struct rt_serial_device serial;
};

enum {
#ifdef BSP_USING_UART1
    USART1_INDEX,
#endif
#ifdef BSP_USING_UART2
    USART2_INDEX,
#endif
#ifdef BSP_USING_UART3
    USART3_INDEX,
#endif
};

USART_Type* uart_instance_table[]={USART1, USART2, USART3, UART4, UART5};

static struct at32_usart usart_config[] = {
#ifdef BSP_USING_UART1
        { "uart1",
        USART1,
        USART1_IRQn, },
#endif
#ifdef BSP_USING_UART2
        { "uart2",
        USART2,
        USART2_IRQn, },
#endif
#ifdef BSP_USING_UART3
        { "uart3",
        USART3,
        USART3_IRQn, },
#endif
};

static rt_err_t at32_configure(struct rt_serial_device *serial,
        struct serial_configure *cfg) 
{
    int32_t instance = -1;
    struct at32_usart *usart_instance = (struct at32_usart *) serial->parent.user_data;

    if(!rt_strcmp(usart_instance->name , "uart1"))
        instance = 0;
    else if(!rt_strcmp(usart_instance->name, "uart2"))
        instance = 1;
    else if(!rt_strcmp(usart_instance->name, "uart3"))
        instance = 2;
    
    
    set_worldlength(instance, cfg->data_bits);

    set_stop_bits(instance, cfg->stop_bits);

    set_parity_bits(instance, cfg->parity);
        
    UART_Init(instance, cfg->baud_rate);
    USART_Cmd(instance, ENABLE);

    return RT_EOK;
}

static rt_err_t at32_control(struct rt_serial_device *serial, int cmd,
        void *arg) 
{
    struct at32_usart *usart;
    uint8_t instance;

    usart = (struct at32_usart *) serial->parent.user_data;

    if(usart->usartx == USART1)
        instance = HW_USART1;
    else if(usart->usartx == USART2)
        instance = HW_USART2;
    else if(usart->usartx == USART3)
        instance = HW_USART3;
    else if(usart->usartx == UART4)
        instance = HW_UART4;
    else if(usart->usartx == UART5)
        instance = HW_UART5;
    
    switch (cmd) 
    {
        case RT_DEVICE_CTRL_CLR_INT:
        
            NVIC_Init(instance,2,1,DISABLE);
            USART_ITConfig(instance, UART_INT_RDNE, DISABLE);
            break;
        case RT_DEVICE_CTRL_SET_INT:
        
            NVIC_Init(instance,2,1,ENABLE);
            USART_ITConfig(instance, UART_INT_RDNE, ENABLE);
            break;
    }

    return RT_EOK;
}

static int at32_putc(struct rt_serial_device *serial, char ch)
{
    struct at32_usart *usart;

    usart = (struct at32_usart *) serial->parent.user_data;

    if(usart != RT_NULL)
    {
        usart->usartx->DT = ch;
        while((usart->usartx->STS & USART_STS_TRAC) == 0);
    }
    return 1;
}

static int at32_getc(struct rt_serial_device *serial) {
    int ch;
    struct at32_usart *usart;

    usart = (struct at32_usart *) serial->parent.user_data;

    ch = -1;

    if (usart->usartx->STS & USART_STS_RDNE)
    {
        ch = (usart->usartx->DT & 0xFF);
        return 1;
    }

    return ch;
}

static const struct rt_uart_ops at32_usart_ops = {
        at32_configure,
        at32_control,
        at32_putc,
        at32_getc,
        RT_NULL };

static void usart_isr(struct rt_serial_device *serial) 
{
    struct at32_usart *usart_instance;

    usart_instance = (struct at32_usart *) serial->parent.user_data;


//    if ((USART_GetITStatus(usart_instance->usartx, UART_INT_RDNE) != RESET) \
//        && (RESET != USART_GetFlagStatus(usart_instance->usartx, USART_FLAG_RDNE))) {
//        rt_hw_serial_isr(serial, RT_SERIAL_EVENT_RX_IND);
//        USART_ClearITPendingBit(usart_instance->usartx, USART_INT_RDNE);
//        USART_ClearFlag(usart_instance->usartx, USART_FLAG_RDNE);
//    } else {
//        if (USART_GetFlagStatus(usart_instance->usartx, USART_FLAG_CTSF) != RESET) {
//            USART_ClearFlag(usart_instance->usartx, USART_FLAG_CTSF);
//        }

//        if (USART_GetFlagStatus(usart_instance->usartx, USART_FLAG_LBDF) != RESET) {
//            USART_ClearFlag(usart_instance->usartx, USART_FLAG_LBDF);
//        }

//        if (USART_GetFlagStatus(usart_instance->usartx, USART_FLAG_TRAC) != RESET) {
//            USART_ClearFlag(usart_instance->usartx, USART_FLAG_TRAC);
//        }
//    }
}

void USART1_IRQHandler(void) 
{
    rt_interrupt_enter();

    usart_isr(&usart_config[USART1_INDEX].serial);

    rt_interrupt_leave();
}


void USART2_IRQHandler(void) 
{
    rt_interrupt_enter();

    usart_isr(&usart_config[USART2_INDEX].serial);

    rt_interrupt_leave();
}



int rt_hw_usart_init(const char *name)
{
    rt_size_t obj_num;
    int index;

    obj_num = sizeof(usart_config) / sizeof(struct at32_usart);
    struct serial_configure config = RT_SERIAL_CONFIG_DEFAULT;
    rt_err_t result = 0;

    for (index = 0; index < obj_num; index++) {
        usart_config[index].serial.ops = &at32_usart_ops;
        usart_config[index].serial.config = config;

        /* register UART device */
        result = rt_hw_serial_register(&usart_config[index].serial,
                usart_config[index].name,
                RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX
                        | RT_DEVICE_FLAG_INT_TX, &usart_config[index]);
        RT_ASSERT(result == RT_EOK);
    }


    return result;
}
INIT_BOARD_EXPORT(rt_hw_usart_init);

#endif /* BSP_USING_SERIAL */
