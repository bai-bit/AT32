#include<common.h>

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
    
    fac_us = (float)GetClock_Frequency(pll) / 8000000;
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

uint32_t GetClock_Frequency(CLOCKFRE_t clock)
{
    uint32_t value = 0, ahb_div = 0, apb1_div = 0, apb2_div = 0;
    uint32_t pll_frafremask = 0, pll_frafreval = 0, pll_value = 0, pll_value1 = 0, pll_value2 = 0;
    
    //��ȡpll�ķ�Ƶֵ��Ȼ����ݷ�Ƶֵ����HSE��Ƶ��
    //HSI��Ƶ��һ��Ϊ8M��Ƶ��
    //PLLһ�����systemcoreclock
    //AHB,APB1,APB2,�Ȼ�ȡRCC->CFG value
    //Ȼ����systemcoreclock���㣬return value
     
    //��Ƶ����
    //��pll_frafremask��ֵ����0x6000 0000�������ֵ����25λ
    //��pll_frafremask��ֵ����0x003c 0000�������ֵ����18λ
    //Ȼ���ϵ�����ֵ��򣬸��ݽ����һ��ȷ����Ƶϵ����
    
    pll_frafremask = (PLL_FRAMASK & RCC->CFG);
    pll_value1 = pll_frafremask & PLL_FRAMASK1;
    pll_value2 = pll_frafremask & PLL_FRAMASK2;
    
    if((pll_value = (pll_value1 | pll_value2)) == RESET)
        pll_frafreval = 2;
    else if(pll_value < 50 && pll_value > 1)
        pll_frafreval = pll_value + 1;
    else
        pll_frafreval = pll_value - 1;
		
    switch(AHB_FRAMASK & RCC->CFG)
    {
        case SYSTEM_1:
            ahb_div = 1;
            break;
        case SYSTEM_2:
            ahb_div = 2;
            break;
        case SYSTEM_4:
            ahb_div = 4;
            break;			
        case SYSTEM_8:
            ahb_div = 8;
            break;
        case SYSTEM_64:
            ahb_div = 64;
            break;
        case SYSTEM_128:
            ahb_div = 128;
            break;
        case SYSTEM_256:
            ahb_div = 256;
            break;
        case SYSTEM_512:
            ahb_div = 512;
    }
    
    switch((APB1_FRAMASK & RCC->CFG) >> 8)
    {
        case HCLK_1:
            apb1_div = 1;
            break;
        case HCLK_2:
            apb1_div = 2;
            break;
        case HCLK_4:
            apb1_div = 4;
            break;
        case HCLK_8:
            apb1_div = 8;
            break;
        case HCLK_16:
            apb1_div = 16;
    }
    switch((APB2_FRAMASK & RCC->CFG) >> 11)
    {
        case HCLK_1:
            apb2_div = 1;
            break;
        case HCLK_2:
            apb2_div = 2;
            break;
        case HCLK_4:
            apb2_div = 4;
            break;
        case HCLK_8:
            apb2_div  = 8;
            break;
        case HCLK_16:
            apb2_div = 16;
    }
    
    switch(clock)
    {
        case pll:
        case systemclock:
            value = SystemCoreClock;
            break;
        case hsi:
            value = 8*1000*1000;
            break;
        case hse:
            value = SystemCoreClock / pll_frafreval;
        case lsi:
            value = 40*1000;
            break;
        case lse:
            value = 32.768*1000;
            break;
        case hclk:
            value = SystemCoreClock / ahb_div;
            break;
        case pclk1:
            value = SystemCoreClock / ahb_div / apb1_div;
            break;
        case pclk2:
            value = SystemCoreClock / ahb_div / apb2_div;
    }
		
    return value;
}

void NVIC_PriorityGroupConfig(uint32_t NVIC_PriorityGroup)
{
	//���ȼ�����
    SCB->AIRCR = NVIC_PriorityGroup | AIRCR_VECTKEY_MASK;
}

void NVIC_Init(uint8_t NVIC_IRQChannel, uint8_t PreemptionPriority, uint8_t SubPriority, FunctionalState NewState)
{
    //��ʼ���жϴ�����
    //����NVIC->IP�Ĵ�����������Ӧ��������ж����ȼ�
    //����NVIC->ISRx�Ĵ�����ʹ����Ӧ�������жϡ�
    uint32_t tempriority = 0, premov = 0, submask = 0x0f;
    
    if(NewState == ENABLE)
    {
        tempriority = (AIRCR_MASK - ((SCB->AIRCR) & AIRCR_MASK)) >> 0x08;
        premov = 0x4 - tempriority;
        submask = submask >> tempriority;
    
        tempriority |= PreemptionPriority << premov;
        tempriority |= SubPriority & submask;
        tempriority = tempriority << 0x04;
    
        NVIC->IP[NVIC_IRQChannel] = tempriority;
		
        NVIC->ISER[NVIC_IRQChannel >> 0x05] = SET << (NVIC_IRQChannel & NVIC_IRQCHAN_MASK);
    }
    else
        NVIC->ICER[NVIC_IRQChannel >> 0x05] = SET << (NVIC_IRQChannel & NVIC_IRQCHAN_MASK);
}
