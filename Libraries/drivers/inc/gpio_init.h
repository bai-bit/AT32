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


#define BITBAND(addr, bitnum) ((addr & 0xF0000000)+0x2000000+((addr &0xFFFFF)<<5)+(bitnum<<2)) 
#define MEM_ADDR(addr)  *((volatile unsigned long  *)(addr)) 
#define BIT_ADDR(addr, bitnum)   MEM_ADDR(BITBAND(addr, bitnum)) 

#define GPIOA_ODR_Addr    (GPIOA_BASE+12)  
#define GPIOB_ODR_Addr    (GPIOB_BASE+12)  
#define GPIOC_ODR_Addr    (GPIOC_BASE+12)  
#define GPIOD_ODR_Addr    (GPIOD_BASE+12) 

#define GPIOA_IDR_Addr    (GPIOA_BASE+8)  
#define GPIOB_IDR_Addr    (GPIOB_BASE+8) 
#define GPIOC_IDR_Addr    (GPIOC_BASE+8) 
#define GPIOD_IDR_Addr    (GPIOD_BASE+8) 

#define PAout(n)   BIT_ADDR(GPIOA_ODR_Addr,n)  
#define PAin(n)    BIT_ADDR(GPIOA_IDR_Addr,n)  
#define PBout(n)   BIT_ADDR(GPIOB_ODR_Addr,n)  
#define PBin(n)    BIT_ADDR(GPIOB_IDR_Addr,n)  
#define PCout(n)   BIT_ADDR(GPIOC_ODR_Addr,n) 
#define PCin(n)    BIT_ADDR(GPIOC_IDR_Addr,n) 
#define PDout(n)   BIT_ADDR(GPIOD_ODR_Addr,n) 
#define PDin(n)    BIT_ADDR(GPIOD_IDR_Addr,n) 

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
