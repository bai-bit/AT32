#ifndef __UART_H
#define __UART_H
#include<gpio_init.h>
#include<at32f4xx.h>
#include<nvic.h>
#include<systemclk.h>
#include<exti.h>
#include<stdio.h>

#define HW_USART1 (0)
#define HW_USART2 (1)
#define HW_USART3 (2)
#define HW_UART4  (3)
#define HW_UART5  (4)


//wordlength,stopbits,parity,mode,hardcontrol
#define wordlength_8  0x0
#define wordlength_9  0x1000

#define stopbits_1    0x0
#define stopbits_0_5  0x1000
#define stopbits_2    0x2000
#define stopbits_1_5  0x3000

#define parity_none   0x0
#define parity_even   0x400
#define parity_odd    0x600

#define mode_rx       0x4
#define mode_tx       0x8

#define hardcontrol_no  0x0
#define hardcontrol_rts 0x100
#define hardcontrol_cts 0x200
#define hardcontrol_all 0x300

#define uart_int_rdne   0x525

#define CTRL1_MASK       0x29f3
#define CTRL2_STOP_Mask  0xcfff
#define CTRL3_Mask       0x4ff

#define REGMASK 0x1f

#define UART_ENABLE     0x2000
#define UART_DATA_MASK  0x1ff

#define UART_SIZE       4096

#define VIRTUAL_BAUD       1000000.00
#define BAUD_9600          9600
#define BAUD_115200        115200
#define BAUD_256000        256000
#define BAUD_460800        460800
#define UART_STATUS_MASK   0x3fff
#define UART_STATUS_ENTER  0x4000
#define UART_STATUS_NLINE  0x8000 

extern char uart_rx_buf[UART_SIZE];
extern uint16_t uart_rx_status;


void uart1_init(uint32_t baud);
void lib_set_brr(uint32_t uartnum,uint32_t baud);
ITStatus uart_getinter(uint32_t uartnum,uint32_t uart_interrupt);
void log_uart(uint8_t uartnum,char *buf);
void call_back_send(uint8_t uartnum,char ch);
uint16_t  uartrecvive_data(uint32_t uartnum);
void USART_Init(uint32_t uartnum,uint32_t baud,uint32_t wordlength,uint32_t stopbits,uint32_t parity,uint32_t mode,uint32_t hardcontrol);
void USART_ITConfig(uint32_t uartnum,uint16_t uart_interrupt,FunctionalState NewStatus);
void USART_Cmd(uint32_t uartnum,FunctionalState NewStatus);

int fgetc(FILE *stream);

#endif
