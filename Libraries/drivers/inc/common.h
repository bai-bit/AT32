#ifndef __HW_SYSTICK_H
#define __HW_SYSTICK_H
#include "at32f4xx.h"

#define SYSTEM_1               0x0000
#define SYSTEM_2               0x0080
#define SYSTEM_4               0x0090
#define SYSTEM_8               0x00A0
#define SYSTEM_16              0x00B0
#define SYSTEM_64              0x00C0
#define SYSTEM_128             0x00D0
#define SYSTEM_256             0x00E0
#define SYSTEM_512             0x00F0
#define	HCLK_1                 0x0000
#define	HCLK_2                 0x0004
#define	HCLK_4                 0x0005
#define	HCLK_8                 0x0006
#define	HCLK_16                0x0007
#define PLL_FRAMASK1           0x60000000
#define PLL_FRAMASK2           0x003c0000
#define SYSTEMSOURCE           0x000c
#define PLL_FRAMASK            0x603C0000
#define AHB_FRAMASK            0x00F0
#define APB1_FRAMASK           0x0700
#define APB2_FRAMASK           0x3800

#define NVIC_PriorityGroup_0   0x700
#define NVIC_PriorityGroup_1   0x600
#define NVIC_PriorityGroup_2   0x500
#define NVIC_PriorityGroup_3   0x400
#define NVIC_PriorityGroup_4   0x300

#define AIRCR_MASK             0x700
#define NVIC_IRQCHAN_MASK      0x1F
#define AIRCR_VECTKEY_MASK     0x05FA0000

#define SYSTICK_SOURCE         SystemCoreClock
#define SYSTICK_EXCEPTION      0x02

#define LIB_DEBUG 
#if defined(LIB_DEBUG)
#include <stdio.h>
#define LIB_TRACE(fmt,args...) \
do{\
    printf("<<File:%s Line:%d Function:%s >>\r\n", __FILE__, __LINE__, __FUNCTION__);\
    printf(fmt,##args);\
}while(0)
#else
#define LIB_TRACE(fmt,args...)
#endif


typedef enum {systemclock,hclk,pclk1,pclk2,hse,hsi,lse,lsi,pll} CLOCKFRE_t;

typedef enum {false = 0,true = !false} bool;

void DelayInit(void);
void systick_setexception(bool val);
void systick_timeout(uint32_t value);
void delayms(uint32_t num);
void delayus(uint32_t num);
uint32_t GetClock_Frequency(CLOCKFRE_t clock);

void NVIC_PriorityGroupConfig(uint32_t NVIC_PriorityGroup);
void NVIC_Init(uint8_t NVIC_IRQChannel,uint8_t PreemptionPriority,uint8_t SubPriority,FunctionalState NewState);

void JumpToImage(uint32_t addr);
#endif
