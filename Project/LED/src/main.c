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
        PAout(9) = 1;
        delayms(5000);
        PAout(9) = 0;
        delayms(5000);
    }
}

void led_red_init(void)
{
    GPIO_Init(HW_GPIOA, GPIO_PIN_9, GPIO_Mode_Out_PP);
    GPIO_PinWrite(HW_GPIOD, GPIO_PIN_9, RESET);
}
