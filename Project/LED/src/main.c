#include "common.h"
#include "systemclk.h"
#include "gpio_init.h"

void led_red_init(void);
int main(int argc, const char *argv[])
{
    SysClk_PLLEN(PLLCLK_MUL_192MHz);
    DelayInit();
    led_red_init();
    
    while(1)
    {
        PDout(13) = 1;
        delayms(500);
        PDout(13) = 0;
        delayms(500);
    }
}

void led_red_init(void)
{
    GPIO_Init(HW_GPIOD, GPIO_PIN_13, GPIO_Mode_Out_PP);
    GPIO_PinWrite(HW_GPIOD, GPIO_PIN_13, RESET);
}
