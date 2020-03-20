#include "at32f4xx.h"
#include "systemclk.h"
#include "common.h"
#include "gpio_init.h"
#include "uart.h"
#include "flash.h"
#include "bl_test.h"
#include <string.h>

#define AppAddress 0x08008000
#define DEVICE_ID 0x00005600

#define PACKET_SIZE 256
#define LENGTH 256

#define BL_TRACE printf

uint8_t flash_erase(uint32_t addr);

void log_uart(void);

bl_test_func_ops_t  bl_test_func = {
    FLASH_SIZE,
    SECTOR_SIZE,
    PACKET_SIZE,
    BOOTLOADER_VERSION,
    AppAddress,
    .reset_mcu = NVIC_SystemReset,
    .bl_send_data = UART_SendData,
    .bl_delayms = delayms,
    .bl_write_memory = Flash_WritePageWord,
    .bl_flash_erase_region = flash_erase
};

void JumpToImage(uint32_t addr);
uint8_t check_app_pc(uint32_t appaddr);
uint8_t jump_app;
int main(int argc, const char *argv[])
{
    SysClk_PLLEN(PLLCLK_MUL_192MHz);

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    DelayInit();
    flash_init();
    
    GPIO_Init(HW_GPIOA, GPIO_PIN_9, GPIO_Mode_AF_PP);
    GPIO_Init(HW_GPIOA, GPIO_PIN_10, GPIO_Mode_IN_FLOATING);
    UART_Init(HW_USART1,   BAUD_115200);
    
    GPIO_Init(HW_GPIOA, GPIO_PIN_2, GPIO_Mode_AF_PP);
    GPIO_Init(HW_GPIOA, GPIO_PIN_3, GPIO_Mode_IN_FLOATING);
    UART_Init(HW_USART2, BAUD_115200);

    framing_packet_t fp_t;
    
    bl_test_init(&bl_test_func);
    systick_timeout(1000);
    systick_setexception(true);

    uint8_t data = 0;
 
    while(1)
    {
        if(UART_GetChar(HW_USART1, &data))
            analysis_packet(&fp_t, data);
        //�ж�pcֵ���Ƿ����0x08008000��С�ڹ涨������ַ
        if(jump_app)
        {
            if(check_app_pc(AppAddress))
            {
                JumpToImage(AppAddress);
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

void log_uart(void)
{
    uint8_t i = 0;
    printf("3");
    for (i = 0;i < 20;i++)
        printf("uart_rx_buf[%d] = [%X]\r\n", i, uart_rx_buf[i]);
}

void SysTick_Handler(void)
{
    static uint32_t timeout = 0;
    if(timeout++ > 300) 
    {
        if(!bl_connect())
            jump_app = 1;
        systick_setexception(false);
    }
}
