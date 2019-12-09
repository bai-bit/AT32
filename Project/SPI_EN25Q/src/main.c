#include<EN25QXXX.h>
#include<systemclk.h>
#include<uart.h>


int main(int argc,const char *argv[])
{
    uint16_t EN25Q_ID = 0;
    sysclk_PLLEN(PLLCLK_MUL_192MHz);
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    DelayInit();
    uart1_init(BAUD_115200);
    EN25QXXX_init(SPI1);
//    GPIO_Init(HW_GPIOA,gpio_pin_4,gpio_speed_50MHz,mode_out_pp);

 
    
    while(1)
    {
//        GPIO_PinWrite(HW_GPIOA,gpio_pin_4,1);
//        delayms(500);
//        GPIO_PinWrite(HW_GPIOA,gpio_pin_4,0);
//        delayms(500);
    //    EN25QXXX_active_mode();
        EN25Q_ID = EN25QXXX_readID(SPI1);
        printf("EN25Q_ID = [%X]\t\n", EN25Q_ID);
        delayms(100);
        while(1);
    }
        
}
