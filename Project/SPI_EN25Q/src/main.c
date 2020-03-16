#include"systemclk.h"
#include"uart.h"
#include"spi_flash.h"
#include"spi.h"
#include"gpio_init.h"
#include"common.h"
#define SIZE 64
#define TEST_ADDR 0x001000
static uint8_t SPIx;
uint8_t ctl_data(uint8_t data);
void ctl_cs(uint8_t status);
void flash_board_module_init(void);

spi_flash_dev_t spi_flash_ops = {
    .xfer_data = ctl_data,
    .cs = ctl_cs
};

int main(int argc,const char *argv[])
{
    SysClk_PLLEN(PLLCLK_MUL_192MHz);
    uint8_t buf[SIZE] = "hello world,welcome to nowyork";  
    uint8_t rbuf[SIZE] = "";
    uint16_t i = 0;
    
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	
    DelayInit();
    
    GPIO_Init(HW_GPIOA, GPIO_PIN_9, GPIO_Mode_AF_PP);
    GPIO_Init(HW_GPIOA, GPIO_PIN_10, GPIO_Mode_IN_FLOATING);
    UART_Init(HW_USART1, BAUD_115200);
    printf("this is applcation program\r\n");
    
    printf("__initial_sp = [%X]\r\n",__get_MSP());
    
    flash_board_module_init();
    
    i = spi_flash_open();
    printf("EN25Q ID = [%X]\r\n",i);

    spi_flash_write(TEST_ADDR, buf, sizeof(buf));
    
    spi_flash_read(TEST_ADDR, rbuf, SIZE);
    
    printf("rbuf3 = [%s]  \r\n",rbuf);
    
    printf("test is over\r\n");
    
    while(1)
        continue;
}

void flash_board_module_init(void)
{
    GPIO_Init(HW_GPIOA, GPIO_PIN_5, GPIO_Mode_AF_PP);
    GPIO_Init(HW_GPIOA, GPIO_PIN_7, GPIO_Mode_AF_PP);
    GPIO_Init(HW_GPIOA, GPIO_PIN_6, GPIO_Mode_IN_FLOATING);
    GPIO_Init(HW_GPIOA, GPIO_PIN_4, GPIO_Mode_Out_PP);
    GPIO_PinWrite(HW_GPIOA, GPIO_PIN_4, 1);
   
    SPI_Init(HW_SPI1, SPI_BAUDRATE_64);
    SPI_SetTR(HW_SPI1, MODE3);
    spi_flash_init(&spi_flash_ops);
}

uint8_t ctl_data(uint8_t data)
{
    return SPI_TransferData(SPIx, data);
}
void ctl_cs(uint8_t status)
{
    PAout(4) = status;
}
