#ifndef __HW_SYSTICK_H
#define __HW_SYSTICK_H
#include<at32f4xx.h>
#include<uart.h>
#include<strfunc.h>
#include<systemclk.h>
#define SYSTICK_SOURCE SystemCoreClock
#define SYSTICK_EXCEPTION 0x02
typedef enum {false = 0,true = !false} bool;

void DelayInit(void);
void systick_setexception(bool val);
void delayms(uint32_t num);
void delayus(uint32_t num);

#endif
