#include "gpio_init.h"

GPIO_Type* gpio_list[]={GPIOA, GPIOB, GPIOC, GPIOD, GPIOE};

void GPIO_Init(uint32_t GPIOx, uint16_t pin, GPIO_t mode)
{
    int pinmode = 0;
    int temp = 0, pinmask = 0;
    
    switch (GPIOx)
    {
        case HW_GPIOA:
            RCC->APB2EN |= RCC_APB2EN_GPIOAEN;
            break;
        case HW_GPIOB:
            RCC->APB2EN |= RCC_APB2EN_GPIOBEN;
            break;
        case HW_GPIOC:
            RCC->APB2EN |= RCC_APB2EN_GPIOCEN;
            break;
        case HW_GPIOD:
            RCC->APB2EN |= RCC_APB2EN_GPIODEN;
            break;
        case HW_GPIOE:
            RCC->APB2EN |= RCC_APB2EN_GPIOEEN;
    }
    
    pinmode = mode & 0xf;
    if ((mode & 0x10) == 0x10)
        pinmode |= GPIO_Speed_2MHz;
    
    if (pin < 8) 
    {
        temp = gpio_list[GPIOx]->CTRLL;
        pin <<= 2;
    
        pinmask = 0xf << pin;
        temp &= ~pinmask;
        temp |= pinmode << pin;
    
        if (mode == GPIO_Mode_IPU)
            gpio_list[GPIOx]->BSRE |= 1 << pin;
        else if (mode == GPIO_Mode_IPD)
            gpio_list[GPIOx]->BRE |= 1 << pin;
      
        gpio_list[GPIOx]->CTRLL = temp;
    }
    else if (pin >= 8 && pin < 16)                                         
    {
        temp = gpio_list[GPIOx]->CTRLH;
        pin = (pin - 8)  << 2;
    
        pinmask = 0xf << pin;
        temp &= ~pinmask;
        temp |= pinmode << pin;
        if (mode == GPIO_Mode_IPU)
            gpio_list[GPIOx]->BSRE |= 1 << pin;
        else if (mode == GPIO_Mode_IPD)
            gpio_list[GPIOx]->BRE |= 1 << pin;
    
        gpio_list[GPIOx]->CTRLH = temp;
    }
}

void GPIO_resetSpeed(uint32_t GPIOx, uint16_t pin, uint8_t speed)
{
    if (pin < 8)
        gpio_list[GPIOx]->CTRLL |= speed << (pin << 2);
    else if (pin >= 8 && pin < 16)
        gpio_list[GPIOx]->CTRLH |= speed << ((pin - 8) << 2);
}
void AFIO_Init(uint32_t GPIOx, uint16_t pin)
{
    //开启AFIO的时钟
    //配置gpio的某个引脚为外部中断的输入源
    //用于触发中断
    RCC->APB2EN |= AFIO_ENABLEBIT;
    
    AFIO->EXTIC[pin >> 2] &= ~(0x01 << ((pin & 0x03) * 0x04));
    AFIO->EXTIC[pin >> 2] |= GPIOx << ((pin & 0x03) * 0x04);
}
void GPIO_PinWrite(uint32_t GPIOx, uint16_t pin, uint8_t num)
{
    if (num)
        gpio_list[GPIOx]->BSRE = 1 << pin;
    else
        gpio_list[GPIOx]->BRE = 1 << pin;
}

void GPIO_PinToggle(uint32_t GPIOx, uint16_t pin)
{
    if (gpio_list[GPIOx]->OPTDT & (0x1 << pin))
        GPIO_PinWrite(GPIOx, pin, 0);
    else
        GPIO_PinWrite(GPIOx, pin, 1);
}

uint16_t read_gpioport(uint32_t GPIOx, uint16_t pin)
{
    //操作GPIOx->IPTDT寄存器
    //return
    return (gpio_list[GPIOx]->IPTDT & (uint16_t)0x01 << pin) ? SET : RESET;
}

void CLKOUT_Init(uint16_t GPIOx, uint16_t pin)
{
    GPIO_Init(GPIOx, pin, GPIO_Mode_AF_PP);
}          


