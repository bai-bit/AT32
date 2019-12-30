#ifndef __HW_SYSTICK_H
#define __HW_SYSTICK_H
#include<at32f4xx.h>

#define SYSTEM_1      0x0000
#define SYSTEM_2      0x0080
#define SYSTEM_4      0x0090
#define SYSTEM_8      0x00A0
#define SYSTEM_16     0x00B0
#define SYSTEM_64     0x00C0
#define SYSTEM_128    0x00D0
#define SYSTEM_256    0x00E0
#define SYSTEM_512    0x00F0
#define	HCLK_1        0x0000
#define	HCLK_2        0x0004
#define	HCLK_4        0x0005
#define	HCLK_8        0x0006
#define	HCLK_16       0x0007
#define PLL_FRAMASK1 0x60000000
#define PLL_FRAMASK2 0x003c0000
#define SYSTEMSOURCE     0x000c
#define PLL_FRAMASK      0x603C0000
#define AHB_FRAMASK      0x00F0
#define APB1_FRAMASK     0x0700
#define APB2_FRAMASK     0x3800

typedef enum {systemclock,hclk,pclk1,pclk2,hse,hsi,lse,lsi,pll} CLOCKFRE_t;

#define SYSTICK_SOURCE SystemCoreClock
#define SYSTICK_EXCEPTION 0x02
typedef enum {false = 0,true = !false} bool;

void DelayInit(void);
void systick_setexception(bool val);
void delayms(uint32_t num);
void delayus(uint32_t num);
uint32_t getclock_frequency(CLOCKFRE_t clock);

#endif
