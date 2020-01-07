#ifndef __SYSTEMCLK_H
#define __SYSTEMCLK_H
#include<at32f4xx.h>

extern uint32_t SystemCoreClock;
#define VECT_TAB_OFFSET     0x0 

#define HSIEN_BIT           0x0001
#define HSEEN_BIT           0x10000
#define PLLEN_BIT           0x01000000

#define HSEEN_Mask          0x10000
#define HSEBYPS_Mask        0x40000
#define PLLEN_Mask          0x2000000

#define HSISTBL_FLAG        0x0002
#define HSESTBL_FLAG        0x20000
#define PLLSTBL_FLAG        0x02000000

#define SYSCLKSEL_HSI       0x0000
#define SYSCLKSEL_HSE       0x0001
#define SYSCLKSEL_PLL       0x0002

#define SYSCLK_Mask         0x0011
#define AHBPSC_Mask         0x00F0
#define APB2PSC_Mask        0x3800
#define APB1PSC_Mask        0x0700

#define SYSCLK_1            0x0000
#define HCLK_1              0x0000

#define LSISTBLF_Mask       0x0001      
#define LSESTBLF_Mask       0x0002
#define HSISTBLF_Mask       0x0004
#define HSESTBLF_Mask       0x0008
#define PLLSTBLF_Mask       0x0010
#define HSECFDF_Mask        0x0080

#define APB1CLK_DIV_2       0x0400
#define APB2CLK_DIV_2       0x2000

#define PLLCLK_Mask         0x603C0000
#define PLLCLK_MUL_16MHz    0x00000000
#define PLLCLK_MUL_96MHz    0x002C0000
#define PLLCLK_MUL_104MHz   0x00300000
#define PLLCLK_MUL_112MHz   0x00340000
#define PLLCLK_MUL_120MHz   0x00380000
#define PLLCLK_MUL_128MHz   0x003c0000
#define PLLCLK_MUL_136MHz   0x20000000
#define PLLCLK_MUL_144MHz   0x20040000
#define PLLCLK_MUL_152MHz   0x20080000
#define PLLCLK_MUL_160MHz   0x200C0000
#define PLLCLK_MUL_168MHz   0x20100000
#define PLLCLK_MUL_176MHz   0x20140000
#define PLLCLK_MUL_184MHz   0x20180000
#define PLLCLK_MUL_192MHz   0x201C0000
#define PLLCLK_MUL_200MHz   0x20200000

#define PLLSRC_HSE          0x10000
#define PLLSRC_HSI          (1 << 16)

#define PLLRANGE_GR72MHz    0x80000000

void SysClk_HSIEN(void);
void SysClk_HSEEN(void);
void SysClk_PLLEN(uint32_t PLLCLK_MUL);
void System_Init(void);
void SystemInit(void);

#endif
