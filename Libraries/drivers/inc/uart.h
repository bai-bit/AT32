#ifndef __UART_H
#define __UART_H
#include "at32f4xx.h"
#include "common.h"
#include <stdio.h>

#define HW_USART1               (0)
#define HW_USART2               (1)
#define HW_USART3               (2)
#define HW_UART4                (3)
#define HW_UART5                (4)

#define VIRTUAL_BAUD            1000000.00
#define BAUD_9600               9600
#define BAUD_115200             115200
#define BAUD_256000             256000
#define BAUD_460800             460800
#define UART_INT_RDNE           0x0525
#define UART_INT_TXE            0x0727
#define CTRL1_MASK              0x29f3
#define CTRL2_STOP_Mask         0xcfff
#define CTRL3_Mask              0x4ff
#define REGMASK                 0x1f
#define UART_DATA_MASK          0x1ff

#define UART_STATUS_MASK        0x3fff
#define UART_STATUS_ENTER       0x4000
#define UART_STATUS_NLINE       0x8000 
#define UART_RX_BUF_SIZE        512
extern char uart_rx_buf[UART_RX_BUF_SIZE];
extern uint16_t uart_rx_status;

typedef enum
{
    kUART_IntTx,
    kUART_IntRx,
}UART_Int_t;

uint32_t UART_Init(uint32_t instance, uint32_t baudrate);
ITStatus UART_GetInter(uint32_t instance, uint32_t uart_interrupt);
uint16_t UART_RecviveData(uint32_t instance);
void USART_ITConfig(uint32_t uartnum,uint16_t uart_interrupt,FunctionalState NewStatus);
void USART_Cmd(uint32_t uartnum,FunctionalState NewStatus);

uint32_t UART_DeInit(uint32_t MAP);
void UART_SetBaudRate(uint32_t instance, uint32_t baud);
uint32_t UART_GetChar(uint32_t instance, uint8_t *ch);
void UART_PutChar(uint32_t instance, uint8_t ch);
uint32_t UART_SetIntMode(uint32_t instance, UART_Int_t mode, uint8_t val);

int fgetc(FILE *stream);
void log_uart(uint8_t uartnum,char *buf);
void call_back_send(uint8_t uartnum,char ch);

#endif
