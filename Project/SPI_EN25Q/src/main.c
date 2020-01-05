#include<EN25QXXX.h>
#include<systemclk.h>
#include<uart.h>
#include<spi_flash.h>
#include<spi.h>
#include<gpio_init.h>
#define SIZE 64
#define TEST_ADDR 0x001000

void ctol_cs(uint8_t status);
void flash_board_module_init(void);

spi_flash_dev_t operation = {
    .open = EN25QXXX_init,
    .read = EN25QXXX_read,
    .write = EN25QXXX_write_page,
    .clear = EN25QXXX_clear,
    .release = EN25QXXX_close,
    .resetbaud = EN25QXXX_baud
};

EN25Q_dev_t EN25Q_ops = {
    .send_data = spi_RWdata,
    .reset_baud = spi_resetbaud,
    .cs = ctol_cs
};

int main(int argc,const char *argv[])
{
    sysclk_PLLEN(PLLCLK_MUL_192MHz);
    uint8_t buf[SIZE] = "hello world,welcome to nowyork";  
    uint8_t rbuf[SIZE] = "";
    uint16_t i = 0;
    
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	
    DelayInit();
    
    GPIO_Init(HW_GPIOA, GPIO_PIN_9,GPIO_Mode_AF_PP);
    GPIO_Init(HW_GPIOA, GPIO_PIN_10,GPIO_Mode_IN_FLOATING);
    UART_Init(HW_USART1,BAUD_115200);
    
    flash_board_module_init();
    
    i = spi_flash_open(&operation);
    printf("EN25Q ID = [%X]\r\n",i);
//    spi_flash_clear(TEST_ADDR);
    spi_flash_write(TEST_ADDR,buf,sizeof(buf));
    
    spi_flash_read(TEST_ADDR,rbuf,SIZE);
    
    printf("rbuf3 = [%s]  \r\n",rbuf);
    
    printf("test is over\r\n");
    
    while(1)
        continue;
}

void flash_board_module_init(void)
{
    GPIO_Init(HW_GPIOA, GPIO_PIN_5,GPIO_Mode_AF_PP);
    GPIO_Init(HW_GPIOA, GPIO_PIN_7,GPIO_Mode_AF_PP);
    GPIO_Init(HW_GPIOA, GPIO_PIN_6,GPIO_Mode_IN_FLOATING);
    GPIO_Init(HW_GPIOA,GPIO_PIN_4,GPIO_Mode_Out_PP);
    GPIO_PinWrite(HW_GPIOA, GPIO_PIN_4, 1);
   
    spi_init(HW_SPI1,SPI_BAUDRATE_64);
    spi_resetTR(HW_SPI1,MODE3);
    EN25Q_module_init(&EN25Q_ops);
}

void ctol_cs(uint8_t status)
{
    PAout(4) = status;
}
