#ifndef __NVIC_H
#define __NVIC_H
#include<at32f4xx.h>

#define AIRCR_VECTKEY_MASK 0x05FA0000

#define NVIC_PriorityGroup_0 0x700
#define NVIC_PriorityGroup_1 0x600
#define NVIC_PriorityGroup_2 0x500
#define NVIC_PriorityGroup_3 0x400
#define NVIC_PriorityGroup_4 0x300

#define AIRCR_MASK 0x700
#define NVIC_IRQCHAN_MASK 0x1F

void NVIC_PriorityGroupConfig(uint32_t NVIC_PriorityGroup);
void NVIC_Init(uint8_t NVIC_IRQChannel,uint8_t PreemptionPriority,uint8_t SubPriority,FunctionalState NewState);

#endif
