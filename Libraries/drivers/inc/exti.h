#ifndef __EXTI_H
#define __EXTI_H
#include<at32f4xx.h>
#include<gpio_init.h>
#include<nvic.h>
#include<uart.h>

#define exti_line0  (0)
#define exti_line1  (1)
#define exti_line2  (2)
#define exti_line3  (3)
#define exti_line4  (4)
#define exti_line5  (5)
#define exti_line6  (6)
#define exti_line7  (7)
#define exti_line8  (8)
#define exti_line9  (9)
#define exti_line10 (10)
#define exti_line11 (11)
#define exti_line12 (12)
#define exti_line13 (13)
#define exti_line14 (14)
#define exti_line15 (15)

typedef enum {interrupt,event} EXTI_MODE;

typedef enum {failling,rising,failling_and_rising} TRIGGER_MODE;

FlagStatus get_extiinter(uint32_t exti_line);
void clean_extiinter(uint32_t exti_line);

void uart1_io_config(void);
void exti_init(uint32_t exti_line,EXTI_MODE exti_mode,TRIGGER_MODE trigger_mode,FunctionalState NewStatus);

#endif
