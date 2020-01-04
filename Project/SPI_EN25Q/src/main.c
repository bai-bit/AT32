#include<EN25QXXX.h>
#include<systemclk.h>
#include<uart.h>
#include<spi_flash.h>
#include<spi.h>
#include<gpio_init.h>

#define SIZE 64
#define TEST_ADDR 695231

void flash_board_module_init(void);

spi_flash_dev_t operation = {
    .open = EN25QXXX_init,
    .read = EN25QXXX_read,
    .write = EN25QXXX_write_data,
    .clear = EN25QXXX_clear,
    .release = EN25QXXX_close,
    .resetbaud = spi_resetbaud
};

EN25Q_dev_t EN25Q_ops = {
    .msg_que = spi_RWdata,
    .cs = GPIO_PinWrite
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
    
    i = spi_flash_open(SPI1,&operation);
    printf("EN25Q ID = [%X]\r\n",i);
    
    spi_flash_read(SPI1,TEST_ADDR,rbuf,SIZE,DATA);
    
    spi_flash_write(SPI1,TEST_ADDR,buf,sizeof(buf),DATA);
    
    spi_flash_read(SPI1,TEST_ADDR,rbuf,SIZE,DATA);
    
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
    
    EN25Q_ops.GPIOx = HW_GPIOA;
    EN25Q_ops.Pin = GPIO_PIN_4;
    spi_init(SPI1,SPI_BAUDRATE_64);
    spi_resetTR(SPI1,MODE3);
    EN25Q_module_init(&EN25Q_ops);
}
