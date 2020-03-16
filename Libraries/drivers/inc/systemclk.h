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

#define  RCC_CFG_PLLMULT2                       ((uint32_t)0x00000000)        /*!< PLL input clock*2 */
#define  RCC_CFG_PLLMULT3                       ((uint32_t)0x00040000)        /*!< PLL input clock*3 */
#define  RCC_CFG_PLLMULT4                       ((uint32_t)0x00080000)        /*!< PLL input clock*4 */
#define  RCC_CFG_PLLMULT5                       ((uint32_t)0x000C0000)        /*!< PLL input clock*5 */
#define  RCC_CFG_PLLMULT6                       ((uint32_t)0x00100000)        /*!< PLL input clock*6 */
#define  RCC_CFG_PLLMULT7                       ((uint32_t)0x00140000)        /*!< PLL input clock*7 */
#define  RCC_CFG_PLLMULT8                       ((uint32_t)0x00180000)        /*!< PLL input clock*8 */
#define  RCC_CFG_PLLMULT9                       ((uint32_t)0x001C0000)        /*!< PLL input clock*9 */
#define  RCC_CFG_PLLMULT10                      ((uint32_t)0x00200000)        /*!< PLL input clock*10 */
#define  RCC_CFG_PLLMULT11                      ((uint32_t)0x00240000)        /*!< PLL input clock*11 */
#define  RCC_CFG_PLLMULT12                      ((uint32_t)0x00280000)        /*!< PLL input clock*12 */
#define  RCC_CFG_PLLMULT13                      ((uint32_t)0x002C0000)        /*!< PLL input clock*13 */
#define  RCC_CFG_PLLMULT14                      ((uint32_t)0x00300000)        /*!< PLL input clock*14 */
#define  RCC_CFG_PLLMULT15                      ((uint32_t)0x00340000)        /*!< PLL input clock*15 */
#define  RCC_CFG_PLLMULT16                      ((uint32_t)0x00380000)        /*!< PLL input clock*16 */

#define  RCC_CFG_PLLMULT17                      ((uint32_t)0x20000000)        /*!< PLL input clock*17 */
#define  RCC_CFG_PLLMULT18                      ((uint32_t)0x20040000)        /*!< PLL input clock*18 */
#define  RCC_CFG_PLLMULT19                      ((uint32_t)0x20080000)        /*!< PLL input clock*19 */
#define  RCC_CFG_PLLMULT20                      ((uint32_t)0x200C0000)        /*!< PLL input clock*20 */
#define  RCC_CFG_PLLMULT21                      ((uint32_t)0x20100000)        /*!< PLL input clock*21 */
#define  RCC_CFG_PLLMULT22                      ((uint32_t)0x20140000)        /*!< PLL input clock*22 */
#define  RCC_CFG_PLLMULT23                      ((uint32_t)0x20180000)        /*!< PLL input clock*23 */
#define  RCC_CFG_PLLMULT24                      ((uint32_t)0x201C0000)        /*!< PLL input clock*24 */
#define  RCC_CFG_PLLMULT25                      ((uint32_t)0x20200000)        /*!< PLL input clock*25 */
#define  RCC_CFG_PLLMULT26                      ((uint32_t)0x20240000)        /*!< PLL input clock*26 */
#define  RCC_CFG_PLLMULT27                      ((uint32_t)0x20280000)        /*!< PLL input clock*27 */
#define  RCC_CFG_PLLMULT28                      ((uint32_t)0x202C0000)        /*!< PLL input clock*28 */
#define  RCC_CFG_PLLMULT29                      ((uint32_t)0x20300000)        /*!< PLL input clock*29 */
#define  RCC_CFG_PLLMULT30                      ((uint32_t)0x20340000)        /*!< PLL input clock*30 */
#define  RCC_CFG_PLLMULT31                      ((uint32_t)0x20380000)        /*!< PLL input clock*31 */
#define  RCC_CFG_PLLMULT32                      ((uint32_t)0x203C0000)        /*!< PLL input clock*32 */
#define  RCC_CFG_PLLMULT33                      ((uint32_t)0x40000000)        /*!< PLL input clock*33 */
#define  RCC_CFG_PLLMULT34                      ((uint32_t)0x40040000)        /*!< PLL input clock*34 */
#define  RCC_CFG_PLLMULT35                      ((uint32_t)0x40080000)        /*!< PLL input clock*35 */
#define  RCC_CFG_PLLMULT36                      ((uint32_t)0x400C0000)        /*!< PLL input clock*36 */
#define  RCC_CFG_PLLMULT37                      ((uint32_t)0x40100000)        /*!< PLL input clock*37 */
#define  RCC_CFG_PLLMULT38                      ((uint32_t)0x40140000)        /*!< PLL input clock*38 */
#define  RCC_CFG_PLLMULT39                      ((uint32_t)0x40180000)        /*!< PLL input clock*39 */
#define  RCC_CFG_PLLMULT40                      ((uint32_t)0x401C0000)        /*!< PLL input clock*40 */
#define  RCC_CFG_PLLMULT41                      ((uint32_t)0x40200000)        /*!< PLL input clock*41 */
#define  RCC_CFG_PLLMULT42                      ((uint32_t)0x40240000)        /*!< PLL input clock*42 */
#define  RCC_CFG_PLLMULT43                      ((uint32_t)0x40280000)        /*!< PLL input clock*43 */
#define  RCC_CFG_PLLMULT44                      ((uint32_t)0x402C0000)        /*!< PLL input clock*44 */
#define  RCC_CFG_PLLMULT45                      ((uint32_t)0x40300000)        /*!< PLL input clock*45 */
#define  RCC_CFG_PLLMULT46                      ((uint32_t)0x40340000)        /*!< PLL input clock*46 */
#define  RCC_CFG_PLLMULT47                      ((uint32_t)0x40380000)        /*!< PLL input clock*47 */
#define  RCC_CFG_PLLMULT48                      ((uint32_t)0x403C0000)        /*!< PLL input clock*48 */
#define  RCC_CFG_PLLMULT49                      ((uint32_t)0x60000000)        /*!< PLL input clock*49 */
#define  RCC_CFG_PLLMULT50                      ((uint32_t)0x60040000)        /*!< PLL input clock*50 */
#define  RCC_CFG_PLLMULT51                      ((uint32_t)0x60080000)        /*!< PLL input clock*51 */
#define  RCC_CFG_PLLMULT52                      ((uint32_t)0x600C0000)        /*!< PLL input clock*52 */
#define  RCC_CFG_PLLMULT53                      ((uint32_t)0x60100000)        /*!< PLL input clock*53 */
#define  RCC_CFG_PLLMULT54                      ((uint32_t)0x60140000)        /*!< PLL input clock*54 */
#define  RCC_CFG_PLLMULT55                      ((uint32_t)0x60180000)        /*!< PLL input clock*55 */
#define  RCC_CFG_PLLMULT56                      ((uint32_t)0x601C0000)        /*!< PLL input clock*56 */
#define  RCC_CFG_PLLMULT57                      ((uint32_t)0x60200000)        /*!< PLL input clock*57 */
#define  RCC_CFG_PLLMULT58                      ((uint32_t)0x60240000)        /*!< PLL input clock*58 */
#define  RCC_CFG_PLLMULT59                      ((uint32_t)0x60280000)        /*!< PLL input clock*59 */
#define  RCC_CFG_PLLMULT60                      ((uint32_t)0x602C0000)        /*!< PLL input clock*60 */
#define  RCC_CFG_PLLMULT61                      ((uint32_t)0x60300000)        /*!< PLL input clock*61 */
#define  RCC_CFG_PLLMULT62                      ((uint32_t)0x60340000)        /*!< PLL input clock*62 */
#define  RCC_CFG_PLLMULT63                      ((uint32_t)0x60380000)        /*!< PLL input clock*63 */
#define  RCC_CFG_PLLMULT64                      ((uint32_t)0x603C0000)        /*!< PLL input clock*64 */

#define PLLSRC_HSE          0x10000
#define PLLSRC_HSI          (1 << 16)

#define PLLRANGE_GR72MHz    0x80000000

void SysClk_HSIEN(void);
void SysClk_HSEEN(void);
void SysClk_PLLEN(uint32_t PLLCLK_MUL);
void System_Init(void);
void SystemInit(void);

#endif
