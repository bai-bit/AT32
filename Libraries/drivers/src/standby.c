#include "standby.h"


void WKUP_Init(void)
{
    //配置寄存器
    RCC->APB1EN |= RCC_APB1EN_PWREN;//使能电源时钟
    PWR->CTRLSTS |= PWR_CTRLSTS_WUPEN;
}

void PWR_EnterSTANDBYMode(void)
{
    GPIOA->CTRLL = 0x0;
    GPIOA->CTRLH = 0x0;
    GPIOB->CTRLL = 0x00200000;
    GPIOB->CTRLH = 0X0;
    GPIOC->CTRLL = 0x0;
    GPIOC->CTRLH = 0x0;
    GPIOD->CTRLL = 0x0;
    GPIOD->CTRLH = 0x0;
    GPIOE->CTRLL = 0x0;
    GPIOE->CTRLH = 0x0;
    
    RCC->APB2EN |= 0x7C;
    
    /* Clear Wake-up flag */
    PWR->CTRL |= PWR_CTRL_CLWUF;
    
    /* Select STANDBY mode */
    PWR->CTRL |= PWR_CTRL_PDDS;
    
    /* Set SLEEPDEEP bit of Cortex System Control Register */
    SCB->SCR |= SCB_SCR_SLEEPDEEP;
    /* This option is used to ensure that store operations are completed */
#if defined ( __CC_ARM   )
    __force_stores();
#endif
    /* Request Wait For Interrupt */
    
    __WFI();
}
