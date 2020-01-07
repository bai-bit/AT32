#ifndef __AUTO_UART_BAUD_H
#define __AUTO_UART_BAUD_H
#include "at32f4xx.h"
#include "gpio_init.h"
#include "exti.h"
#include "uart.h"

#define SIZE          8
#define BYTE_Mask     9
#define RELOAD_VALUE  1920000

void auto_uart1_baud(void);

#endif
