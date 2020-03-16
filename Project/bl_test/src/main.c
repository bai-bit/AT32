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
#include "flash.h"
#include <string.h>

#define P 0x50
#define AppAddress 0x08004000
#define SIZE 32
#define BOOTLOADER_VERSION 0x4b010500
#define DEVICE_ID 0x00005600

#define PACKET_SIZE 256
#define LENGTH 256

void JumpToImage(uint32_t addr);
uint16_t crc_update(const uint8_t *src, uint32_t LengthInBytes);
void clear_uart_rx_buf(void);
void command_function(void);
void recv_data(void);
void ping_response_packet(void);


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
    kFramingPacketType_StartByte = 0x5A,
    kFramingPacketType_Ack = 0xA1,
    kFramingPacketType_Nak = 0xA2,
    kFramingPacketType_AckAbort = 0xA3,
    kFramingPacketType_Command = 0xA4,
    kFramingPacketType_Data = 0xA5,
    kFramingPacketType_Ping = 0xA6,
    kFramingPacketType_PingResponse = 0xA7
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
uint16_t check_crc(framing_packet_t *fp_t, const uint16_t length);

void analysis_command_tag(framing_packet_t *fp_t);
void property_tag(framing_packet_t *fp_t);
void command_response_packet(framing_packet_t *fp_t, uint32_t value);

static uint8_t count = 0;
static uint16_t length = 0;
static uint8_t status = KStatus_Start;

int main(int argc, const char *argv[])
{
    SysClk_PLLEN(PLLCLK_MUL_192MHz);
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    DelayInit();
    flash_init();
    GPIO_Init(HW_GPIOA, GPIO_PIN_9, GPIO_Mode_AF_PP);
    GPIO_Init(HW_GPIOA, GPIO_PIN_10, GPIO_Mode_IN_FLOATING);
    UART_Init(HW_USART1,   BAUD_115200);
//    printf("this is bootloader\r\n");
//    printf("__initial_sp = [%X]\r\n",__get_MSP());
    framing_packet_t fp_t;
    uint8_t data = 0;
      
    while(1)
    {
        if(UART_GetChar(HW_USART1, &data))
            analysis_packet(&fp_t, data);
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
                    break;
                    
                case kFramingPacketType_AckAbort:
                    break;
                    
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
                    
                    ping_response_packet();
                    status = KStatus_Start;
                    break;
                case kFramingPacketType_PingResponse:
                    fp_t->packet_type = kFramingPacketType_PingResponse;
                    status = KStatus_Data;
                    break;
                default:
                    status = KStatus_Start;
                    
            }
            break;
            
        case KStatus_LenLow:
            
            status = KStatus_LenHigh;
            fp_t->len_low = data;
            break;
        case KStatus_LenHigh:
            fp_t->len_high = data;
            length = 0;
            length |= 0xff & fp_t->len_low ;
            length |= ((uint16_t)(fp_t->len_high)) << 8;
       
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
            fp_t->data[count++] = data;
            if (count == length)
            {
                status = KStatus_Start;
                count = 0;
                check_crc(fp_t,length);
            }
            break;
        default:
            status = KStatus_Start;
    }
}

uint16_t check_crc(framing_packet_t *fp_t,const uint16_t length)
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
    {
        switch(fp_t->packet_type)
        {
            case kFramingPacketType_Command:
                analysis_command_tag(fp_t);
                break;
            case kFramingPacketType_Data:
                break;
        }
    }
    else
    {
        printf("%d",length);
        printf("%X",fp_t->crc_low);
        printf("%X",fp_t->crc_high);
        printf("%X",crc_value);
        printf("%X",ret);
    }
    return ret;
}

void analysis_command_tag(framing_packet_t *fp_t)
{
    switch(fp_t->data[0])
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
                property_tag(fp_t);
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

void property_tag(framing_packet_t *fp_t)
{
    //检索标签
    //检查第九位的数值，判断参数的个数
    //每四个字节为一个参数
    switch(fp_t->data[4])
    {
        case 0x01:
            //调用发送bootloader版本号的函数
            command_response_packet(fp_t, BOOTLOADER_VERSION);
            break;
//        case 0x02:
//        case 0x03:
        case 0x04:
            command_response_packet(fp_t, FLASH_SIZE);
            break;
        case 0x05:
            command_response_packet(fp_t, SECTOR_SIZE);
            break;
//        case 0x06:
//        case 0x07:
//        case 0x0A:
        case 0x0B:
            command_response_packet(fp_t, PACKET_SIZE);
            break;
        case 0x10:
            command_response_packet(fp_t, DEVICE_ID);
    }        
}

void command_response_packet(framing_packet_t *fp_t, uint32_t value)
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
    command_buf[0] = fp_t->start_byte;
    command_buf[1] = fp_t->packet_type;
    command_buf[2] = 0x0c;
    command_buf[3] = 0;
    command_buf[4] = kFramingPacketType_PingResponse;
    command_buf[7] = 0x02;
    
    ret = crc_update(command_buf,16);
    buf[4] = (uint8_t)(ret & 0xff);
    buf[5] = (uint8_t)((ret & 0xff00) >> 8);
    
    for(i = 0; i < 4; i++)
        buf[i] = command_buf[i];
    for(i = 6; i < 18; i++)
        buf[i] = command_buf[i-2];
    UART_SendData(HW_USART1, buf, sizeof(buf));
    memset(fp_t,0,sizeof(framing_packet_t));
    
}


void ping_response_packet(void)
{
    uint8_t src[10] = "";
    uint16_t ret = 0;
    
    src[0] = kFramingPacketType_StartByte;
    src[1] = kFramingPacketType_PingResponse;
    src[3] = 0x02;
    src[4] = 0x01;
    src[5] = P;

    ret = crc_update(src,8);
    src[8] = ret & 0x00ff;
    src[9] = (ret & 0xff00) >> 8;
    delayms(1);
    UART_SendData(HW_USART1,src,10);
}


void recv_data(void)
{
    
}


void flash_erase_region(void)
{
    //擦除指定的地址和范围
    //只进行擦除操作，从指定的地址处开始擦除指定的字节数
    
}

void write_memory(void)
{
    //在指定的地址处写入指定的字节数
    //
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



void log_uart(void)
{
    uint8_t i = 0;
    printf("3");
    for (i = 0;i < 20;i++)
    {
        printf("uart_rx_buf[%d] = [%X]\r\n", i, uart_rx_buf[i]);
    }
    
}

