//这里实现一个功能，就是jump to image
//过程：
//  开启时钟
//  开启中断优先级分组
//  开启自动匹配串口，通过发送0x5a
//  然后指定一个地址，然后将sp初始值修改为这个指定的地址
//  然后在这个指定的地址上，运行另外一段程序，例如读出flash中的一段数据
//  在跳转之前先在指定的地址上下载好目标程序
//  怎么在指定的地址上下载
//  可以通过keil自带的功能，指定烧写地址
//  可以通过一个软件，用串口发送数据


#include "at32f4xx.h"
#include "systemclk.h"
#include "common.h"
#include "gpio_init.h"
#include "uart.h"
#include <string.h>

#define AppAddress 0x08004000
#define SIZE 32
#define BOOTLOADER_VERSION 0x4b010500
#define DEVICE_ID 0x00005600
#define FLASH_SIZE 0x00040000
#define SECTOR_SIZE 1024
#define PACKET_SIZE 256
#define LENGTH 256

void JumpToImage(uint32_t addr);
uint16_t crc_update(const uint8_t *src, uint32_t LengthInBytes);
void clear_uart_rx_buf(void);
void command_function(void);
void recv_data(void);
void ping_response_packet(void);
void property_tag(void);

void ack_packet(void);
void log_uart(void);
void flash_erase_region(void);
void write_memory(void);
void fill_memory(void);
void flash_security_disable(void);
enum status {
    KStatus_Start,
    KStatus_Cmd,
    KStatus_LenLow,
    KStatus_LenHigh,
    KStatus_CrcLow,
    KStatus_CrcHigh,
    KStatus_Data
};

enum {
    kFramingPacketType_StartByte = 0x5a,
    kFramingPacketType_Ack = 0xa1,
    kFramingPacketType_Nak = 0xa2,
    kFramingPacketType_AckAbort = 0xa3,
    kFramingPacketType_Command = 0xa4,
    kFramingPacketType_Data = 0xa5,
    kFramingPacketType_Ping = 0xa6,
    kFramingPacketType_PingResponse = 0xa7
};

typedef struct framing_packet {
    uint8_t start_byte;
    uint8_t packet_type;
    uint8_t len_low;
    uint8_t len_high;
    uint8_t crc_low;
    uint8_t crc_high;
    uint8_t data[LENGTH];
}framing_packet_t;
void analysis_packet(framing_packet_t *fp_t, uint8_t data);
uint16_t check_crc(framing_packet_t *fp_t, uint16_t length);
void command_response_packet(uint32_t value);

int main(int argc, const char *argv[])
{
    SysClk_PLLEN(PLLCLK_MUL_192MHz);
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    DelayInit();
    GPIO_Init(HW_GPIOA, GPIO_PIN_9, GPIO_Mode_AF_PP);
    GPIO_Init(HW_GPIOA, GPIO_PIN_10, GPIO_Mode_IN_FLOATING);
    UART_Init(HW_USART1,   BAUD_115200);
//    printf("this is bootloader\r\n");
//    printf("__initial_sp = [%X]\r\n",__get_MSP());
    USART_ITConfig(HW_USART1, UART_INT_RDNE, DISABLE);
//    USART_Cmd(HW_USART1, DISABLE);
    
    NVIC_Init(USART1_IRQn, 2, 2, DISABLE);

    
    framing_packet_t fp_t;
    uint8_t data = 0;
    uint8_t rx[32] = "";
    uint8_t i = 0;
   
    while(1)
    {
        if(UART_GetChar(HW_USART1, &data))
            analysis_packet(&fp_t, data);

//        if((uart_rx_buf[0] == 0x5a) && (uart_rx_buf[1] == 0xa6))
//        {   
//            ping_response_packet();
//        }
//        else if(uart_rx_status > 1)
//        {
//            memset(uart_rx_buf,0,sizeof(uart_rx_buf));
//            uart_rx_status = RESET;
//        }
    }
//    JumpToImage(AppAddress);
}

void JumpToImage(uint32_t addr)
{
    uint32_t *vector_table = (uint32_t *)addr;
    uint32_t sp = vector_table[0];
    uint32_t pc = vector_table[1];
    typedef void(*applcation_entry)(void);
    applcation_entry app_address = (applcation_entry) pc;

    __set_MSP(sp);
    __set_PSP(sp);
     
    app_address();
}

uint16_t crc_update(const uint8_t *src, uint32_t LengthInBytes)
{
    uint32_t crc = 0;
    uint32_t j = 0;
    for (j = 0; j < LengthInBytes; j++)
    {
        uint32_t i;
        uint32_t byte = src[j];
        crc ^= byte << 8;
        for (i = 0; i < 8; i++)
        {
            uint32_t temp = crc << 1;
            if (crc & 0x8000)
                temp ^= 0x1021;
            crc = temp;
        }
    }
    return crc;
}




void analysis_packet(framing_packet_t *fp_t, uint8_t data)
{
    static uint8_t count = 0;
    static uint16_t length = 0;
    static uint8_t status = KStatus_Start;
    
    switch(status)
    {
        case KStatus_Start:
            if(kFramingPacketType_StartByte == data)
            {
                status = KStatus_Cmd;
                fp_t->start_byte = data;
            }
            
            break;
        case KStatus_Cmd:
            switch(data)
            {
                case kFramingPacketType_Ack:
                    status = KStatus_Start;
                    break;
                case kFramingPacketType_Nak:
                    
                case kFramingPacketType_AckAbort:
                    
                case kFramingPacketType_Command:
                    fp_t->packet_type = kFramingPacketType_Command;
                    status = KStatus_LenLow;
                    break;
                case kFramingPacketType_Data:
                    fp_t->packet_type = kFramingPacketType_Data;
                    status = KStatus_LenLow;
                    
                    break;
                case kFramingPacketType_Ping:
                    fp_t->packet_type = kFramingPacketType_Ping;
                    status = KStatus_Start;
                    printf("123");
                    clear_uart_rx_buf();
                    break;
                case kFramingPacketType_PingResponse:
                    fp_t->packet_type = kFramingPacketType_PingResponse;
                    status = KStatus_Data;
                    
            }
            break;
        case KStatus_LenLow:
            
            status = KStatus_LenHigh;
            fp_t->len_low = data;
//            printf("low = %d",fp_t->len_low);
            break;
        case KStatus_LenHigh:
            
            fp_t->len_high = data;
//            printf("high = %d",data);
            length |= 0xff & fp_t->len_low ;
            length |= ((uint16_t)(fp_t->len_high)) << 8;
//            printf(" length = %d",length);
       
            if(length <= LENGTH)
                status = KStatus_CrcLow;
            else 
                status = KStatus_Start;//应该回复上位机一个消息，告诉它消息太长
            
            break;
        case KStatus_CrcLow:
            
            status = KStatus_CrcHigh;
            fp_t->crc_low = data;
            break;
        case KStatus_CrcHigh:
            status = KStatus_Data;
            fp_t->crc_high = data;
            break;
        case KStatus_Data:
            //判断包类型
            //命令包和数据包要根据length值进行限制接收
            //ping包发送应答信号
        
            fp_t->data[count++] = data;
            if (count == length)
            {
                delayms(500);
                //计算crc，将包头和包尾放入一个数组中
                status = KStatus_Start;
                count = 0;
                check_crc(fp_t,length);
            }
    }
}

uint16_t check_crc(framing_packet_t *fp_t, uint16_t length)
{
    uint8_t packet_buf[LENGTH + 4] = "";
    uint16_t i = 0;
    uint16_t ret = 0;
    uint16_t crc_value = 0;
    
    packet_buf[0] = fp_t->start_byte;
    packet_buf[1] = fp_t->packet_type;
    packet_buf[2] = fp_t->len_low;
    packet_buf[3] = fp_t->len_high;
    for (i = 0; i < length; i++)
        packet_buf[i + 4] = fp_t->data[i];
    
    ret = crc_update(packet_buf, length + 4);
    crc_value |= 0xff & fp_t->crc_low;
    crc_value |= (((uint16_t)fp_t->crc_high) << 8) & 0xff00;

    if(ret == crc_value)
        analysis_command_tag(fp_t->data);
        //进一步解析command tag
    //执行analysis tag函数
    //对不同的需求进行处理
}

void analysis_command_tag(framing_packet_t *fp_t)
{
    
}
static uint8_t a = 0;

void ping_response_packet(void)
{
    uint8_t src[10] = "";
    uint16_t ret = 0;
    
    src[0] = uart_rx_buf[0];
    src[1] = 0xa7;
    src[3] = 0x02;
    src[4] = 0x01;
    src[5] = 0x50;

    ret = crc_update(src,8);
    src[8] = ret & 0x00ff;
    src[9] = (ret & 0xff00) >> 8;
    UART_SendData(HW_USART1,src,10);
            
    memset(uart_rx_buf,0,sizeof(uart_rx_buf));
    uart_rx_status = RESET;
    
    while(1)
    {
       
        if(0x5a == uart_rx_buf[a])
        {
            if(0xa6 == uart_rx_buf[a + 1])
            {
                ping_response_packet();
                break;
            }
            
            switch(uart_rx_buf[a + 1])
            {
                case 0xa4:
                    //调用处理命令
                    command_function();
                case 0xa5:
                    //调用接收数据
                    recv_data();
            }                    
        }
        if(0x5a == uart_rx_buf[0] && 0xa1 == uart_rx_buf[1])
            a = 2;
        else 
            a = 0;
    }
}
void command_function(void)
{
    //对接收的命令进行判断
    uint16_t length = 0;
    length |= 0xff & uart_rx_buf[a + 2];
    length |= 0xff00 & ((uint16_t)uart_rx_buf[a + 3] << 8);
    uint16_t i = 0;
    uint8_t buf[SIZE] = "";
    
    buf[0] = uart_rx_buf[a + 0];
    buf[1] = uart_rx_buf[a + 1];
    buf[2] = uart_rx_buf[a + 2];
    buf[3] = uart_rx_buf[a + 3];
    
    for (i = 4; i < length + 4; i++)
    {
        //接收信息，信息长度被限制
        buf[i] = uart_rx_buf[2 + a + i]; 
    }
    
    //计算crc
    //调用一个函数进行整理信息，放入一个数组，进行计算crc
    uint16_t ret = 0;
    ret = crc_update(buf, i);
    uint16_t crc_value =0;
    crc_value |= 0xff & uart_rx_buf[a + 4];
    crc_value |= 0xff00 & ((uint16_t)uart_rx_buf[a + 5] << 8);
       
    if(ret == crc_value)
    {
        switch(uart_rx_buf[a + 6])
        {
            case 0x02:          //FlashEraseRegion
                flash_erase_region();
                break;
            case 0x04:          //WriteMemory
                write_memory();
                break;
            case 0x05:          //FillMemory
                fill_memory();
                break;
            case 0x06:          //FlashSecurityDisable
                flash_security_disable();
                break;
            case 0x07:          //GetProperty
                property_tag();
                break;
//            case 0x09:
//            case 0x10:
//            case 0x0B:
//            case 0x0C:
//            case 0x0D:
//            case 0x0E:
//            case 0x0F:
                
        }
    }
}

void recv_data(void)
{
    
}

void property_tag(void)
{
    //检索标签
    //检查第九位的数值，判断参数的个数
    //每四个字节为一个参数
    switch(uart_rx_buf[a + 10])
    {
        case 0x01:
            //调用发送bootloader版本号的函数
            command_response_packet(BOOTLOADER_VERSION);
            break;
//        case 0x02:
//        case 0x03:
        case 0x04:
            command_response_packet(FLASH_SIZE);
            break;
        case 0x05:
            command_response_packet(SECTOR_SIZE);
            break;
//        case 0x06:
//        case 0x07:
//        case 0x0A:
        case 0x0B:
            command_response_packet(PACKET_SIZE);
            break;
        case 0x10:
            command_response_packet(DEVICE_ID);
    }        
}



void flash_erase_region(void)
{
    
    //擦除扇区
    //解析数据包的末尾两个包，其实地址和擦除的字节数量
//    uint32_t addr = 0;
//    addr |= (uint32_t)tuart_rx_buf[10];
//    addr |= (((uint32_t)uart_rx_buf[11]) << 8);
//    addr |= 
//    Flash_ErasePage(addr);
}

void write_memory(void)
{
    //写数据
}
void fill_memory(void)
{
    
}
void flash_security_disable(void)
{
    
}

void ack_packet(void)
{
    uint8_t ping_buf[] = {0x5A, 0xA1};
    UART_SendData(HW_USART1, ping_buf, 2);
}

void command_response_packet(uint32_t value)
{
    delayms(1);
    ack_packet();
    delayms(1);
    uint8_t buf[18] = "";
    uint8_t command_buf[16] = "";
    uint16_t ret = 0;
    uint8_t i = 0;
    if(value)
    {
        command_buf[12] = (uint8_t)(value & 0xFF);
        command_buf[13] = (uint8_t)((value >> 8) & 0xFF);
        command_buf[14] = (uint8_t)((value >> 16) & 0xFF);
        command_buf[15] = (uint8_t)((value >> 24) & 0xFF);
    }
    command_buf[0] = uart_rx_buf[a];
    command_buf[1] = uart_rx_buf[a + 1];
    command_buf[2] = 0x0c;
    command_buf[3] = 0;
    command_buf[4] = 0xa7;
    command_buf[7] = 0x02;
    
    ret = crc_update(command_buf,16);
    buf[5] = (uint8_t)(ret & 0xff);
    buf[6] = (uint8_t)((ret >> 8) & 0xff);
    
    for(i = 0; i < 4; i++)
        buf[i] = command_buf[i];
    for(i = 6; i < 18; i++)
        buf[i] = command_buf[i-2];
    UART_SendData(HW_USART1, buf, sizeof(buf));
    memset(uart_rx_buf,0,18);
    uart_rx_status = RESET;
}

void log_uart(void)
{
    uint8_t i = 0;
    printf("3");
    for (i = 0;i < 20;i++)
    {
        printf("uart_rx_buf[%d] = [%X]\r\n", i, uart_rx_buf[i]);
    }
    
}