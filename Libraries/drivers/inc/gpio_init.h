#ifndef __GPIO_INIT_H
#define __GPIO_INIT_H

#include <at32f4xx.h>

#define HW_GPIOA  (0)
#define HW_GPIOB  (1)
#define HW_GPIOC  (2)
#define HW_GPIOD  (3)
#define HW_GPIOE  (4)

#define gpio_pin_0  (0)
#define gpio_pin_1  (1)
#define gpio_pin_2  (2)
#define gpio_pin_3  (3)
#define gpio_pin_4  (4)
#define gpio_pin_5  (5)
#define gpio_pin_6  (6)
#define gpio_pin_7  (7)
#define gpio_pin_8  (8)
#define gpio_pin_9  (9)
#define gpio_pin_10 (10)
#define gpio_pin_11 (11)
#define gpio_pin_12 (12)
#define gpio_pin_13 (13)
#define gpio_pin_14 (14)
#define gpio_pin_15 (15)


#define AFIO_ENABLEBIT 0x01

typedef enum  {
	mode_in_analog =     0x00,
	mode_in_floating =   0x04,
	mode_in_pd =         0x28,
	mode_in_pu =         0x48,
	mode_out_pp =        0x10,
	mode_out_od =        0x14,
	mode_af_pp =         0x18,
	mode_af_od =         0x1c
}gpiomode;

typedef enum  {
	gpio_speed_inno =  0,
	gpio_speed_10MHz = 1,
	gpio_speed_50MHz = 2,
	gpio_speed_2MHz
}gpiospeed;

void GPIO_Init(uint32_t GPIOx, uint16_t pin, gpiospeed speed, gpiomode mode);
void GPIO_PinWrite(uint32_t GPIOx, uint16_t pin, uint32_t num);
void GPIO_PinToggle(uint32_t GPIOx, uint16_t pin);
uint16_t read_gpioport(uint32_t GPIOx, uint16_t pin);
void write_gpioport(uint32_t GPIOx, uint16_t pin, uint8_t data);
void AFIO_Init(uint32_t GPIOx, uint16_t pin);
void CLKOUT_Init(uint16_t GPIOx,uint16_t pin);

#endif
