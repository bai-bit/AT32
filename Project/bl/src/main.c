#include "at32f4xx.h"
#include "systemclk.h"
#include "common.h"
#include "gpio_init.h"
#include "uart.h"
#include "flash.h"
#include "bl.h"
#include <string.h>

#define APP_ADDRESS         (0x08008000)
#define DEVICE_ID           (0x00005600)
#define PACKET_SIZE         (256)

#define BL_TRACE printf

uint8_t flash_erase(uint32_t addr);

bl_test_func_ops_t  bl_test_func = {
    FLASH_SIZE,
    SECTOR_SIZE,
    PACKET_SIZE,
    BOOTLOADER_VERSION,
    APP_ADDRESS,
    .reset_mcu = NVIC_SystemReset,
    .bl_send_data = UART_SendData,
    .bl_delayms = delayms,
    .bl_write_memory = Flash_WritePageWord,
    .bl_flash_erase_region = flash_erase
};

uint8_t jump_app;

int main(int argc, const char *argv[])
{
    SysClk_HSIEN();

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    DelayInit();
    flash_init();
  
    /* mux UART? to ..TC:PA PB.... */
    RCC->APB2EN |= AFIO_ENABLEBIT;
    AFIO->MAP |= AFIO_MAP_USART1_REMAP ;
    GPIO_Init(HW_GPIOB, GPIO_PIN_6, GPIO_Mode_AF_PP);
    GPIO_Init(HW_GPIOB, GPIO_PIN_7, GPIO_Mode_IN_FLOATING);
//    GPIO_Init(HW_GPIOA, GPIO_PIN_9, GPIO_Mode_AF_PP);
//    GPIO_Init(HW_GPIOA, GPIO_PIN_10, GPIO_Mode_IN_FLOATING);
    
    UART_Init(HW_USART1, BAUD_115200);
    
    GPIO_Init(HW_GPIOA, GPIO_PIN_2, GPIO_Mode_AF_PP);
    GPIO_Init(HW_GPIOA, GPIO_PIN_3, GPIO_Mode_IN_FLOATING);
    
    UART_Init(HW_USART2, BAUD_115200);
    GPIO_Init(HW_GPIOA,GPIO_PIN_9,GPIO_Mode_Out_PP);
    PAout(9) = 1;

    framing_packet_t fp_t;
    
    bl_init(&bl_test_func);
    systick_timeout(1000);
    systick_setexception(true);

    uint8_t data = 0;
 
    while(1)
    {
        if(UART_GetChar(HW_USART1, &data))
            analysis_packet(&fp_t, data);
        //判断pc值，是否大于0x08008000，小于规定的最大地址
        if(jump_app)
        {
            if(check_app_pc(APP_ADDRESS))
            {
                JumpToImage(APP_ADDRESS);
            }
        }
    }
}

uint8_t flash_erase(uint32_t addr)
{
    FLASH_Status status = FLASH_COMPLETE;
    status = Flash_ErasePage(addr);
    
    if(status == FLASH_COMPLETE)
        return 0;
    return 1;
}

void SysTick_Handler(void)
{
    static uint32_t timeout = 0;
    if(timeout++ > 400) 
    {
        if(!bl_connect())
            jump_app = 1;
        systick_setexception(false);
    }
}

