#ifndef __VIRTUAL_UART_H
#define __VIRTUAL_UART_H
#include "gpio_init.h"
#include "exti.h"
#include "at32f4xx.h"
#include "uart.h"

void uart1_io_config(void);
void uart_virtual(uint32_t baud);
void uartsend_data(uint32_t uartnum);
void writedata(uint32_t uartnum,uint16_t data);
void virtual_serial(uint32_t uartnum,uint16_t data);
uint8_t uart_recvive_data(uint32_t uartnum);
uint16_t  readdata(uint32_t uartnum);

#endif
