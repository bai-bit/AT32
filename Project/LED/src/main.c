#include "common.h"
#include "systemclk.h"
#include "gpio_init.h"

void led_red_init(void);
int main(int argc, const char *argv[])
{
    SysClk_PLLEN(PLLCLK_MUL_192MHz);
//    SysClk_HSEEN();
    DelayInit();
    led_red_init();
    
    while(1)
    {
        PDout(13) = 1;
        PDout(14) = 1;
        PDout(15) = 1;
        delayms(1000);
        PDout(13) = 0;
        PDout(14) = 1;
        PDout(15) = 1;
        delayms(1000);
    }
}

void led_red_init(void)
{
    GPIO_Init(HW_GPIOD, GPIO_PIN_13, GPIO_Mode_Out_PP);
    GPIO_PinWrite(HW_GPIOD, GPIO_PIN_13, RESET);
    GPIO_Init(HW_GPIOD, GPIO_PIN_14, GPIO_Mode_Out_PP);
    GPIO_PinWrite(HW_GPIOD, GPIO_PIN_14, RESET);
    GPIO_Init(HW_GPIOD, GPIO_PIN_15, GPIO_Mode_Out_PP);
    GPIO_PinWrite(HW_GPIOD, GPIO_PIN_15, RESET);
}
