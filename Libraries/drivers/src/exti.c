//外部中断
#include<exti.h>

extern u8 uart_read_buf[UART_SIZE];
extern uint16_t uart_read_status;

void Exti_Init(uint32_t exti_line, EXTI_MODE exti_mode, TRIGGER_MODE trigger_mode, FunctionalState NewStatus)
{
	//使能某个中断线
	//配置触发方式
	if(NewStatus)
	{
		exti_mode ? (EXTI->EVTEN |= SET << exti_line) : (EXTI->INTEN |= SET << exti_line);
		if(trigger_mode == failling)
			EXTI->FTRSEL |= SET << exti_line;
		else if(trigger_mode == rising)
			EXTI->RTRSEL |= SET << exti_line;
		else
		{
			EXTI->FTRSEL |= SET << exti_line;
			EXTI->RTRSEL |= SET << exti_line;
		}
	}
	else
		exti_mode ? (EXTI->INTEN &= ~(SET << exti_line)) : (EXTI->EVTEN &= ~(SET << exti_line));
}

FlagStatus Get_ExtiInter(uint32_t exti_line)
{
	return (EXTI->PND | (0x02 << exti_line))? SET : RESET;
}

void Clean_ExtiInter(uint32_t exti_line)
{
	EXTI->PND |= 0x02 << exti_line;
}






