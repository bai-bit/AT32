#include<systemclk.h>

uint32_t SystemCoreClock;

//开启HSE

void sysclk_HSIEN(void)
{
	system_init();
	RCC->CFG |= SYSCLKSEL_HSI;
	RCC->CFG |= SYSCLK_1 | HCLK_1;
	SystemCoreClock = 8000000;
}

void sysclk_HSEEN(void)
{
	//先使能外部高速时钟
	//切换系统时钟源
	//关闭内部高速时钟
	system_init();
	RCC->CTRL |= HSEEN_BIT;
	
	do{
		
	}while((RCC->CTRL & HSESTBL_FLAG) == 0 );
	
	RCC->CFG |= SYSCLK_1 | HCLK_1;
		
	RCC->CFG |= SYSCLKSEL_HSE;
	RCC->CTRL &= ~HSIEN_BIT;
	SystemCoreClock = 8000000;

}

void sysclk_PLLEN(uint32_t PLLCLK_MUL)
{
	//使能HSE时钟
	//稳定之后，确定各总线的分频系数
	//选择PLL的倍频系数
	//使能PLL时钟
	
	system_init();
	RCC->CTRL |= HSEEN_BIT;
	
	do{
		
	}while((RCC->CTRL & HSESTBL_FLAG) == 0);
	
	RCC->CFG |= SYSCLK_1;
	RCC->CFG |= APB1CLK_DIV_2;
	RCC->CFG |= APB2CLK_DIV_2;
	
	RCC->CFG &= ~PLLCLK_Mask;
	RCC->CFG |= PLLCLK_MUL;
	RCC->CFG |= PLLSRC_HSE;
//RCC->CFG &= ~PLLSRC_HSI;

	RCC->CFG |= PLLRANGE_GR72MHz;
	
	RCC->CTRL |= PLLEN_BIT;
	do{
		
	}while((RCC->CTRL & PLLSTBL_FLAG) == 0);
	
	RCC->CFG |= SYSCLKSEL_PLL;
	switch(PLLCLK_MUL)
	{
		case PLLCLK_MUL_16MHz:
			SystemCoreClock = 16000000;
			break;
		case PLLCLK_MUL_96MHz:
			SystemCoreClock = 96000000;
			break;
		case PLLCLK_MUL_104MHz:
			SystemCoreClock = 104000000;
			break;
		case PLLCLK_MUL_112MHz: 
			SystemCoreClock = 112000000;
			break;
		case PLLCLK_MUL_120MHz:
			SystemCoreClock = 120000000;
			break;
		case PLLCLK_MUL_128MHz:
			SystemCoreClock = 128000000;
			break;
		case PLLCLK_MUL_136MHz:
			SystemCoreClock = 136000000;
			break;
		case PLLCLK_MUL_144MHz:
			SystemCoreClock = 144000000;
			break;
		case PLLCLK_MUL_152MHz:
			SystemCoreClock = 152000000;
			break;
		case PLLCLK_MUL_160MHz:
			SystemCoreClock = 160000000;
			break;
		case PLLCLK_MUL_168MHz:
			SystemCoreClock = 168000000;
			break;
		case PLLCLK_MUL_176MHz:
			SystemCoreClock = 176000000;
			break;
		case PLLCLK_MUL_184MHz:
			SystemCoreClock = 184000000;
			break;
		case PLLCLK_MUL_192MHz:
			SystemCoreClock = 192000000;
			break;
		case PLLCLK_MUL_200MHz:
			SystemCoreClock = 200000000;
			break;
	}
	
}

void system_init(void)
{
	//先使能HSI
	RCC->CTRL |= HSIEN_BIT;
	//清除一些配置位
	//RESET SYSCLKSEL AHBPSC APB1PSC APB2PSC
	//RESET HSEEN PLLEN HSEBYPS 
	//RESET LSISTBLFC LSE HSI HSE PLL HSECFDFC
	RCC->CFG &= ~(SYSCLK_Mask | AHBPSC_Mask | APB1PSC_Mask | APB2PSC_Mask);
	RCC->CTRL &= ~(HSEEN_Mask | HSEBYPS_Mask | PLLEN_Mask);

}

void SystemInit(void)
{
#if defined (__FPU_USED) && (__FPU_USED == 1U)
  SCB->CPACR |= ((3U << 10U * 2U) |        
                 (3U << 11U * 2U)  ); 
#endif
	
#ifdef VECT_TAB_SRAM
  SCB->VTOR = SRAM_BASE | VECT_TAB_OFFSET; /* Vector Table Relocation in Internal SRAM. */
#else
  SCB->VTOR = FLASH_BASE | VECT_TAB_OFFSET; /* Vector Table Relocation in Internal FLASH. */
#endif
}


//输入clock，返回对应的clock frequency
//思路：先获取systemcoreclock frequency
//然后读取相应的寄存器配置值
//然后用switch。。。case，根据clock和RCC->CFG的value
//将systemcoreclock 分频，return value


uint32_t getclock_frequency(CLOCKFRE_t clock)
{
	uint32_t value = 0, ahb_div = 0, apb1_div = 0, apb2_div = 0;
	uint32_t pll_frafremask = 0, pll_frafreval = 0, pll_value = 0, pll_value1 = 0, pll_value2 = 0;
	
	//获取pll的分频值，然后根据分频值返回HSE的频率
	//HSI的频率一般为8M的频率
	//PLL一般等于systemcoreclock
	//AHB,APB1,APB2,先获取RCC->CFG value
	//然后用systemcoreclock计算，return value
	pll_frafremask = (PLL_FRAMASK & RCC->CFG); 
	//分频计算
	//将pll_frafremask的值与上0x6000 0000；把这个值右移25位
	//将pll_frafremask的值与上0x003c 0000；把这个值右移18位
	//然后将上的两个值相或，根据结果加一来确定分频系数。
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
