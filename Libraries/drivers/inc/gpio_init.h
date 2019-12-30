#ifndef __GPIO_INIT_H
#define __GPIO_INIT_H

#include <at32f4xx.h>

#define HW_GPIOA  (0)
#define HW_GPIOB  (1)
#define HW_GPIOC  (2)
#define HW_GPIOD  (3)
#define HW_GPIOE  (4)

#define GPIO_PIN_0  (0)
#define GPIO_PIN_1  (1)
#define GPIO_PIN_2  (2)
#define GPIO_PIN_3  (3)
#define GPIO_PIN_4  (4)
#define GPIO_PIN_5  (5)
#define GPIO_PIN_6  (6)
#define GPIO_PIN_7  (7)
#define GPIO_PIN_8  (8)
#define GPIO_PIN_9  (9)
#define GPIO_PIN_10 (10)
#define GPIO_PIN_11 (11)
#define GPIO_PIN_12 (12)
#define GPIO_PIN_13 (13)
#define GPIO_PIN_14 (14)
#define GPIO_PIN_15 (15)


#define AFIO_ENABLEBIT 0x01

//IO口操作宏定义
#define BITBAND(addr, bitnum) ((addr & 0xF0000000)+0x2000000+((addr &0xFFFFF)<<5)+(bitnum<<2)) 
#define MEM_ADDR(addr)  *((volatile unsigned long  *)(addr)) 
#define BIT_ADDR(addr, bitnum)   MEM_ADDR(BITBAND(addr, bitnum)) 
//IO口地址映射
#define GPIOA_ODR_Addr    (GPIOA_BASE+12) //0x4001080C 
#define GPIOB_ODR_Addr    (GPIOB_BASE+12) //0x40010C0C 
#define GPIOC_ODR_Addr    (GPIOC_BASE+12) //0x4001100C 
#define GPIOD_ODR_Addr    (GPIOD_BASE+12) //0x4001140C 
#define GPIOE_ODR_Addr    (GPIOE_BASE+12) //0x4001180C 
#define GPIOF_ODR_Addr    (GPIOF_BASE+12) //0x40011A0C    
#define GPIOG_ODR_Addr    (GPIOG_BASE+12) //0x40011E0C    

#define GPIOA_IDR_Addr    (GPIOA_BASE+8) //0x40010808 
#define GPIOB_IDR_Addr    (GPIOB_BASE+8) //0x40010C08 
#define GPIOC_IDR_Addr    (GPIOC_BASE+8) //0x40011008 
#define GPIOD_IDR_Addr    (GPIOD_BASE+8) //0x40011408 
#define GPIOE_IDR_Addr    (GPIOE_BASE+8) //0x40011808 
#define GPIOF_IDR_Addr    (GPIOF_BASE+8) //0x40011A08 
#define GPIOG_IDR_Addr    (GPIOG_BASE+8) //0x40011E08 
 
//IO口操作,只对单一的IO口!
//确保n的值小于16!
#define PAout(n)   BIT_ADDR(GPIOA_ODR_Addr,n)  //输出 
#define PAin(n)    BIT_ADDR(GPIOA_IDR_Addr,n)  //输入 

#define PBout(n)   BIT_ADDR(GPIOB_ODR_Addr,n)  //输出 
#define PBin(n)    BIT_ADDR(GPIOB_IDR_Addr,n)  //输入 

#define PCout(n)   BIT_ADDR(GPIOC_ODR_Addr,n)  //输出 
#define PCin(n)    BIT_ADDR(GPIOC_IDR_Addr,n)  //输入 

#define PDout(n)   BIT_ADDR(GPIOD_ODR_Addr,n)  //输出 
#define PDin(n)    BIT_ADDR(GPIOD_IDR_Addr,n)  //输入 

#define PEout(n)   BIT_ADDR(GPIOE_ODR_Addr,n)  //输出 
#define PEin(n)    BIT_ADDR(GPIOE_IDR_Addr,n)  //输入

#define PFout(n)   BIT_ADDR(GPIOF_ODR_Addr,n)  //输出 
#define PFin(n)    BIT_ADDR(GPIOF_IDR_Addr,n)  //输入

#define PGout(n)   BIT_ADDR(GPIOG_ODR_Addr,n)  //输出 
#define PGin(n)    BIT_ADDR(GPIOG_IDR_Addr,n)  //输入

//以下为汇编函数
void WFI_SET(void);		//执行WFI指令
void INTX_DISABLE(void);//关闭所有中断
void INTX_ENABLE(void);	//开启所有中断
void MSR_MSP(u32 addr);	//设置堆栈地址

typedef enum  {
	GPIO_Mode_AIN           =   0x00,
	GPIO_Mode_IN_FLOATING   =   0x04,
	GPIO_Mode_IPD           =   0x28,
	GPIO_Mode_IPU           =   0x48,
	GPIO_Mode_Out_PP        =   0x10,
	GPIO_Mode_Out_OD        =   0x14,
	GPIO_Mode_AF_PP         =   0x18,
	GPIO_Mode_AF_OD         =   0x1c
}GPIO_t;

typedef enum  {
	GPIO_Speed_INNO  = 0,
	GPIO_Speed_10MHz = 1,
	GPIO_Speed_50MHz,
	GPIO_Speed_2MHz,
}GPIOSpeed;

void GPIO_Init(uint32_t GPIOx, uint16_t pin, GPIOSpeed speed, GPIO_t mode);
void GPIO_PinWrite(uint32_t GPIOx, uint16_t pin, uint8_t num);
void GPIO_PinToggle(uint32_t GPIOx, uint16_t pin);
uint16_t read_gpioport(uint32_t GPIOx, uint16_t pin);
void write_gpioport(uint32_t GPIOx, uint16_t pin, uint8_t data);
void AFIO_Init(uint32_t GPIOx, uint16_t pin);
void CLKOUT_Init(uint16_t GPIOx,uint16_t pin);

#endif
