#ifndef __SYSTEMCLK_H
#define __SYSTEMCLK_H
#include<at32f4xx.h>

extern uint32_t SystemCoreClock;
#define VECT_TAB_OFFSET  0x0 

#define HSIEN_BIT      0x0001
#define HSEEN_BIT      0x10000
#define PLLEN_BIT      0x01000000

#define HSEEN_Mask     0x10000
#define HSEBYPS_Mask   0x40000
#define PLLEN_Mask     0x2000000

#define HSISTBL_FLAG   0x0002
#define HSESTBL_FLAG   0x20000
#define PLLSTBL_FLAG   0x02000000

#define SYSCLKSEL_HSI  0x0000
#define SYSCLKSEL_HSE  0x0001
#define SYSCLKSEL_PLL  0x0002

#define SYSCLK_Mask    0x0011
#define AHBPSC_Mask    0x00F0
#define APB2PSC_Mask   0x3800
#define APB1PSC_Mask   0x0700

#define SYSCLK_1       0x0000
#define HCLK_1         0x0000

#define LSISTBLF_Mask  0x0001      
#define LSESTBLF_Mask  0x0002
#define HSISTBLF_Mask  0x0004
#define HSESTBLF_Mask  0x0008
#define PLLSTBLF_Mask  0x0010
#define HSECFDF_Mask   0x0080

#define APB1CLK_DIV_2  0x0400
#define APB2CLK_DIV_2  0x2000

#define PLLCLK_Mask        0x603C0000
#define PLLCLK_MUL_16MHz   0x00000000
#define PLLCLK_MUL_96MHz   0x002C0000
#define PLLCLK_MUL_104MHz  0x00300000
#define PLLCLK_MUL_112MHz  0x00340000
#define PLLCLK_MUL_120MHz  0x00380000
#define PLLCLK_MUL_128MHz  0x003c0000
#define PLLCLK_MUL_136MHz  0x20000000
#define PLLCLK_MUL_144MHz  0x20040000
#define PLLCLK_MUL_152MHz  0x20080000
#define PLLCLK_MUL_160MHz  0x200C0000
#define PLLCLK_MUL_168MHz  0x20100000
#define PLLCLK_MUL_176MHz  0x20140000
#define PLLCLK_MUL_184MHz  0x20180000
#define PLLCLK_MUL_192MHz  0x201C0000
#define PLLCLK_MUL_200MHz  0x20200000

#define PLLSRC_HSE     0x10000
#define PLLSRC_HSI     (1 << 16)

#define PLLRANGE_GR72MHz 0x80000000

typedef enum {systemclock,hclk,pclk1,pclk2,hse,hsi,lse,lsi,pll} CLOCKFRE_t;

#define SYSTEMSOURCE     0x000c
#define PLL_FRAMASK      0x603C0000
#define AHB_FRAMASK      0x00F0
#define APB1_FRAMASK     0x0700
#define APB2_FRAMASK     0x3800

#define SYSTEM_1      0x0000
#define SYSTEM_2      0x0080
#define SYSTEM_4      0x0090
#define SYSTEM_8      0x00A0
#define SYSTEM_16     0x00B0
#define SYSTEM_64     0x00C0
#define SYSTEM_128    0x00D0
#define SYSTEM_256    0x00E0
#define SYSTEM_512    0x00F0
#define	HCLK_1        0x0000
#define	HCLK_2        0x0004
#define	HCLK_4        0x0005
#define	HCLK_8        0x0006
#define	HCLK_16       0x0007
#define PLL_FRAMASK1 0x60000000
#define PLL_FRAMASK2 0x003c0000

uint32_t getclock_frequency(CLOCKFRE_t clock);
void sysclk_HSIEN(void);
void sysclk_HSEEN(void);
void sysclk_PLLEN(uint32_t PLLCLK_MUL);
void system_init(void);
void SystemInit(void);

#endif
