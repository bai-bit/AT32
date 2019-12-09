#include<systick.h>

static __IO float fac_us;
static __IO float fac_ms;


void DelayInit(void)
{
	//�����ڲ�ʱ��Դ��Ƶ��
	//��ʼ��systick��ʱ��
	//ѡ��ʱ��Դ
	//�ô����hclkʱ��Դ,�����ms��us��������
	//��ʼ��fac_ms,fac_us.
	SysTick->CTRL &= ~(SysTick_CTRL_CLKSOURCE_Msk);
	
	fac_us = (float)getclock_frequency(pll) / 8000000;
	fac_ms = fac_us * 1000;
}

void delayms(uint32_t num)
{
	//������װ�ؼĴ���ֻ��24λ����������,���Դ����������̫��,
	//�����������ݹ���,��Ҫ�ֶ��д����װ�ؼĴ���.
	//�������ֵ
	uint32_t temp = 0;
	SysTick->LOAD = fac_us * 1000;
	SysTick->VAL = SysTick->LOAD;
	SysTick->CTRL |=  SysTick_CTRL_ENABLE_Msk;
	while((num)--)
	{
		do{
				temp = SysTick->CTRL;
			}while((temp & SysTick_CTRL_ENABLE_Msk ) && !(temp & SysTick_CTRL_COUNTFLAG_Msk));
	}
	SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
}

void delayus(uint32_t num)
{
	//�Ѵ������ʱʱ�����fac_usд����װ�ؼĴ���
	//������ʱ��
	//��do...while()��������ѭ�����жϿ��ƼĴ����ĵ�ʮ��λ
	//����ʮ��λΪ1ʱ,�رն�ʱ��.
	
	uint32_t temp = 0;
	
	SysTick->LOAD = (num) * fac_us;
	SysTick->VAL = 0;
	SysTick->CTRL |=  SysTick_CTRL_ENABLE_Msk;
	do{
		 temp = SysTick->CTRL;
    }while((temp & SysTick_CTRL_ENABLE_Msk ) && !(temp & SysTick_CTRL_COUNTFLAG_Msk));
	
	SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
}


/*
** @brief   enable or disable systick
** @note    none
** @param   val:
**          @arg  true    enable systick exception request
**          @arg  false   disable systick exception request
** @retval  none
*/
void systick_setexception(bool val)
{
	val == true ? (SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk) : (SysTick->CTRL &= ~(SysTick_CTRL_TICKINT_Msk)); 
}



