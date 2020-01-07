#include<common.h>

static __IO float fac_us;
static __IO float fac_ms;


void DelayInit(void)
{
    //传入内部时钟源的频率
    //初始化systick定时器
    //选择时钟源
    //用传入的hclk时钟源,计算出ms和us的周期数
    //初始化fac_ms,fac_us.
    SysTick->CTRL &= ~(SysTick_CTRL_CLKSOURCE_Msk);
    
    fac_us = (float)GetClock_Frequency(pll) / 8000000;
    fac_ms = fac_us * 1000;
}

void delayms(uint32_t num)
{
    //由于重装载寄存器只有24位来保存数据,所以传入的数不能太大,
    //如果传入的数据过大,需要分多次写入重装载寄存器.
    //计算最大值
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
    //把传入的延时时间乘以fac_us写入重装载寄存器
    //开启定时器
    //用do...while()出口条件循环来判断控制寄存器的第十六位
    //当第十六位为1时,关闭定时器.
    
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
    
    //获取pll的分频值，然后根据分频值返回HSE的频率
    //HSI的频率一般为8M的频率
    //PLL一般等于systemcoreclock
    //AHB,APB1,APB2,先获取RCC->CFG value
    //然后用systemcoreclock计算，return value
     
    //分频计算
    //将pll_frafremask的值与上0x6000 0000；把这个值右移25位
    //将pll_frafremask的值与上0x003c 0000；把这个值右移18位
    //然后将上的两个值相或，根据结果加一来确定分频系数。
    
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
	//优先级分组
    SCB->AIRCR = NVIC_PriorityGroup | AIRCR_VECTKEY_MASK;
}

void NVIC_Init(uint8_t NVIC_IRQChannel, uint8_t PreemptionPriority, uint8_t SubPriority, FunctionalState NewState)
{
    //初始化中断处理函数
    //配置NVIC->IP寄存器，设置相应的外设的中断优先级
    //配置NVIC->ISRx寄存器，使能相应的外设中断。
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
