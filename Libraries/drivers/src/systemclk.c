#include<systemclk.h>

uint32_t SystemCoreClock;

//����HSE

void SysClk_HSIEN(void)
{
	System_Init();
	RCC->CFG |= SYSCLKSEL_HSI;
	RCC->CFG |= SYSCLK_1 | HCLK_1;
	SystemCoreClock = 8000000;
}

void SysClk_HSEEN(void)
{
	//��ʹ���ⲿ����ʱ��
	//�л�ϵͳʱ��Դ
	//�ر��ڲ�����ʱ��
	System_Init();
	RCC->CTRL |= HSEEN_BIT;
	
	do{
		
	}while((RCC->CTRL & HSESTBL_FLAG) == 0 );
	
	RCC->CFG |= SYSCLK_1 | HCLK_1;
		
	RCC->CFG |= SYSCLKSEL_HSE;
	RCC->CTRL &= ~HSIEN_BIT;
	SystemCoreClock = 8000000;

}

void SysClk_PLLEN(uint32_t PLLCLK_MUL)
{
	//ʹ��HSEʱ��
	//�ȶ�֮��ȷ�������ߵķ�Ƶϵ��
	//ѡ��PLL�ı�Ƶϵ��
	//ʹ��PLLʱ��
	
	System_Init();
	RCC->CTRL |= HSEEN_BIT;
	
	do{
		
	}while((RCC->CTRL & HSESTBL_FLAG) == 0);
	
	RCC->CFG |= SYSCLK_1;
	RCC->CFG |= APB1CLK_DIV_2;
	RCC->CFG |= APB2CLK_DIV_2;
	
	RCC->CFG &= ~PLLCLK_Mask;
	RCC->CFG |= PLLCLK_MUL;
	RCC->CFG |= PLLSRC_HSE;

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

void System_Init(void)
{
	//��ʹ��HSI
	RCC->CTRL |= HSIEN_BIT;
	//���һЩ����λ
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
