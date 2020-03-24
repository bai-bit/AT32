#include "bl.h"

#define FLASH_ERASE_REGION          (0x02)
#define WRITE_MEMORY                (0x04)
#define GET_PROPERTY                (0x07)
#define RESET_MCU                   (0x0B)
#define FILL_MEMORY                 (0x05)
#define FLASH_SECURITY_DISABLE      (0x06)
            
#define CURRENT_VERSION             (0x01)
#define FLASH_SIZE_IN_BYTES         (0x04)
#define FLASH_SECTOR_SIZE           (0x05)
#define SYSTEM_DEVICELD             (0x10)
#define MAX_PACKET_SIZE             (0x0B)
#define AVAILABLE_PERIPHERALS       (0x02)
#define FLASH_START_ADDRESS         (0x03)
#define FLASH_BLOCK_COUNT           (0x06)
#define AVAILABLE_COMMANDS          (0x07)
#define VETRIFY_WRITES              (0x0A)

typedef struct bl_env_variate {
    uint16_t count;
    uint16_t length;
    uint8_t status;
    uint8_t app_flag;
    uint32_t write_memory_start;
}bl_env_variate_t;

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

typedef struct generic_response_packer {
    uint8_t start_type;
    uint8_t packet_type; 
    uint8_t len_low;
    uint8_t len_high;
    uint8_t crc_low;
    uint8_t crc_high;
    uint8_t response_tag;
    uint8_t response_count;
    uint32_t response_status;
    uint32_t command_tag;
}generic_response_packet_t;

typedef struct app_program_location {
    uint32_t flash_start_addr;
    uint32_t flash_byte_count;
}app_program_location_t;

static bl_env_variate_t bl_variate;

static generic_response_packet_t generic_packet;

static app_program_location_t app_program;

static bl_test_func_ops_t bl_remap_ops;

static uint16_t check_crc(framing_packet_t *fp_t, const uint16_t len);
static void flash_erase_region(framing_packet_t *fp_t);
static void analysis_command_tag(framing_packet_t *fp_t);
static void property_tag(framing_packet_t *fp_t);
static void command_response_packet(framing_packet_t *fp_t, uint32_t value);
static void generic_response_packet(framing_packet_t *fp_t, uint8_t status);
static void write_memory(framing_packet_t *fp_t);
static void recv_data(framing_packet_t *fp_t);
static uint8_t reset_mcu(framing_packet_t *fp_t);
static void analysis_app_download_addr(framing_packet_t *fp_t);
static void ping_response_packet(void);
static void ack_packet(void);

void bl_init(bl_test_func_ops_t *bl_remap)
{
    memcpy(&bl_remap_ops,bl_remap, sizeof(bl_remap_ops));
}

uint16_t crc_update(const uint8_t *src, uint32_t len)
{
    uint32_t crc = 0;
    uint32_t j = 0;
    for (j = 0; j < len; j++)
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
    switch(bl_variate.status)
    {
        case KStatus_Start:
            if(kFramingPacketType_StartByte == data)
            {
                bl_variate.app_flag = 1;
                bl_variate.status = KStatus_Cmd;
                fp_t->start_byte = data;
            }
            break;
        case KStatus_Cmd:
            switch(data)
            {
                case kFramingPacketType_Ack:
                    bl_variate.status = KStatus_Start;
                    break;
                case kFramingPacketType_Nak:
                    break;
                    
                case kFramingPacketType_AckAbort:
                    break;
                    
                case kFramingPacketType_Command:
                    fp_t->packet_type = kFramingPacketType_Command;
                    bl_variate.status = KStatus_LenLow;
                    break;
                case kFramingPacketType_Data:
                    
                    fp_t->packet_type = kFramingPacketType_Data;
                    bl_variate.status = KStatus_LenLow;
                    
                    break;
                case kFramingPacketType_Ping:
                    fp_t->packet_type = kFramingPacketType_Ping;
                    
                    ping_response_packet();
                    bl_variate.status = KStatus_Start;
                    break;
                case kFramingPacketType_PingResponse:
                    fp_t->packet_type = kFramingPacketType_PingResponse;
                    bl_variate.status = KStatus_Data;
                    break;
                default:
                    bl_variate.status = KStatus_Start;
                    
            }
            break;
            
        case KStatus_LenLow:
            
            bl_variate.status = KStatus_LenHigh;
            fp_t->len_low = data;
            break;
        case KStatus_LenHigh:
            fp_t->len_high = data;
            bl_variate.length = 0;
            bl_variate.length |= 0xff & fp_t->len_low ;
            bl_variate.length |= ((uint16_t)(fp_t->len_high)) << 8;
       
            if(bl_variate.length <= bl_remap_ops.packet_size)
            {
                bl_variate.status = KStatus_CrcLow;
            }
            else 
                bl_variate.status = KStatus_Start;//Ӧ�ûظ���λ��һ����Ϣ����������Ϣ̫��
            break;
        case KStatus_CrcLow:
            bl_variate.status = KStatus_CrcHigh;
            fp_t->crc_low = data;
            break;
        case KStatus_CrcHigh:
            bl_variate.status = KStatus_Data;
            fp_t->crc_high = data;
            break;
        case KStatus_Data:
            fp_t->data[bl_variate.count++] = data;
            if (bl_variate.count == bl_variate.length)
            {
                
                bl_variate.status = KStatus_Start;
                bl_variate.count = 0;
                check_crc(fp_t, bl_variate.length);
            }
            break;
        default:
            bl_variate.status = KStatus_Start;
    }
}

static uint16_t check_crc(framing_packet_t *fp_t, const uint16_t len)
{
    uint8_t packet_buf[LENGTH + 4] = "";
    uint16_t i = 0;
    uint16_t ret = 0;
    uint16_t crc_value = 0;
    
    packet_buf[0] = fp_t->start_byte;
    packet_buf[1] = fp_t->packet_type;
    packet_buf[2] = fp_t->len_low;
    packet_buf[3] = fp_t->len_high;
    for (i = 0; i < len; i++)
        packet_buf[i + 4] = fp_t->data[i];
    
    ret = crc_update(packet_buf, len + 4);
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
                recv_data(fp_t);
                break;
            default:
                BL_TRACE("fp_t->packet_type = [%X]\r\n", fp_t->packet_type);
        }
    }
    else                                                //DEBUG
    {
        printf("%d", len);
        printf("%X", fp_t->crc_low);
        printf("%X", fp_t->crc_high);
        printf("%X", crc_value);
        printf("%X", ret);
    }
    return ret;
}

static void analysis_command_tag(framing_packet_t *fp_t)
{
    switch(fp_t->data[0])
        {
            case FLASH_ERASE_REGION:                    //FlashEraseRegion
                flash_erase_region(fp_t);
                break;
            case WRITE_MEMORY:                          //WriteMemory
                write_memory(fp_t);
                break;
//            case FILL_MEMORY:                         //FillMemory 
//                fill_memory();
//                break;
//            case FLASH_SECURITY_DISABLE:              //FlashSecurityDisable
//                flash_security_disable();
//                break;
            case GET_PROPERTY:                          //GetProperty
                property_tag(fp_t);
                break;

//            case 0x09:
//            case 0x10:
            case RESET_MCU:                             //ResetMcu
                reset_mcu(fp_t);
                break;
//            case 0x0C:
//            case 0x0D:
//            case 0x0E:
//            case 0x0F:
            default:
                BL_TRACE("fp_t->data[0] = [%X]\r\n", fp_t->data[0]);
        }
}

static uint8_t reset_mcu(framing_packet_t *fp_t)
{
    ack_packet();
    generic_packet.packet_type = kFramingPacketType_Command;
    generic_packet.command_tag = RESET_MCU;
    generic_response_packet(fp_t, 0);
    bl_remap_ops.reset_mcu();
    
    return 1;
}

static void property_tag(framing_packet_t *fp_t)
{
    switch(fp_t->data[4])
    {
        case CURRENT_VERSION :
            //���÷���bootloader�汾�ŵĺ���
            command_response_packet(fp_t, BOOTLOADER_VERSION);
            break;
//        case AVAILABLE_PERIPHERALS:
//        case FLASH_START_ADDRESS:
        case FLASH_SIZE_IN_BYTES:
            command_response_packet(fp_t, bl_remap_ops.flash_size);
            break;
        case FLASH_SECTOR_SIZE:
            command_response_packet(fp_t, bl_remap_ops.flash_sector_size);
            break;
//        case FLASH_BLOCK_COUNT:
//        case AVAILABLE_COMMANDS:
//        case VETRIFY_WRITES:
        case MAX_PACKET_SIZE:
            command_response_packet(fp_t, bl_remap_ops.packet_size);
            break;
        case SYSTEM_DEVICELD:
            command_response_packet(fp_t, bl_remap_ops.device_id);
            break;
        default:
            BL_TRACE("fp_t->data[4] = [%X]\r\n", fp_t->data[4]);
    }        
}

static void command_response_packet(framing_packet_t *fp_t, uint32_t value)
{
    ack_packet();

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
    
    ret = crc_update(command_buf, 16);
    buf[4] = (uint8_t)(ret & 0xff);
    buf[5] = (uint8_t)((ret & 0xff00) >> 8);
    
    for(i = 0; i < 4; i++)
        buf[i] = command_buf[i];
    for(i = 6; i < 18; i++)
        buf[i] = command_buf[i-2];
    bl_remap_ops.bl_send_data(0, buf, sizeof(buf));
    memset(fp_t,0,sizeof(framing_packet_t));
}

static void ping_response_packet(void)
{
    uint8_t buf[10] = "";
    uint16_t ret = 0;
    
    buf[0] = kFramingPacketType_StartByte;
    buf[1] = kFramingPacketType_PingResponse;
    buf[3] = 0x02;
    buf[4] = 0x01;
    buf[5] = PINT_RESPONSE_PACKET_P;

    ret = crc_update(buf, 8);
    
    buf[8] = ret & 0x00ff;
    buf[9] = (ret & 0xff00) >> 8;
    
    bl_remap_ops.bl_delayms(1);
    
    bl_remap_ops.bl_send_data(0,buf,10);
}

static void recv_data(framing_packet_t *fp_t)
{
    uint16_t length = fp_t->len_low | (uint16_t)fp_t->len_high << 8;

    if(bl_variate.write_memory_start >= bl_remap_ops.app_address)
    {
        bl_remap_ops.bl_write_memory(bl_variate.write_memory_start, (uint16_t *)fp_t->data, length / 2);
        bl_variate.write_memory_start += length;
    }
    
    ack_packet();
    
    if(bl_variate.write_memory_start == app_program.flash_byte_count + app_program.flash_start_addr)
    {
        bl_variate.write_memory_start = 0;
   
        generic_packet.packet_type = kFramingPacketType_Command;
        generic_packet.command_tag = WRITE_MEMORY ;
        generic_response_packet(fp_t,0);
    }
}

static void flash_erase_region(framing_packet_t *fp_t)
{
    uint16_t sector_num = 0;
    uint32_t secremain = 0;
    uint32_t i = 0;
    
    ack_packet();
    analysis_app_download_addr(fp_t);
    sector_num = app_program.flash_byte_count / bl_remap_ops.flash_sector_size;
    secremain = app_program.flash_byte_count % bl_remap_ops.flash_sector_size;
    if(secremain)
        sector_num++;

    for(i = 0; i < sector_num; i++)
        bl_remap_ops.bl_flash_erase_region(app_program.flash_start_addr + bl_remap_ops.flash_sector_size * i);

    generic_packet.packet_type = fp_t->packet_type;
    generic_packet.command_tag = fp_t->data[0] & 0xFF;
    generic_response_packet(fp_t, 0);
}

static void analysis_app_download_addr(framing_packet_t *fp_t)
{
    app_program.flash_start_addr |= fp_t->data[4];
    app_program.flash_start_addr |= (uint16_t)fp_t->data[5] << 8;
    app_program.flash_start_addr |= (uint32_t)fp_t->data[6] << 16;
    app_program.flash_start_addr |= (uint32_t)fp_t->data[7] << 24;
    app_program.flash_byte_count |= fp_t->data[8];
    app_program.flash_byte_count |= (uint16_t)fp_t->data[9] << 8;
    app_program.flash_byte_count |= (uint32_t)fp_t->data[10] << 16;
    app_program.flash_byte_count |= (uint32_t)fp_t->data[11] << 24;
}

static void generic_response_packet(framing_packet_t *fp_t, uint8_t status)
{
    uint8_t buf[18] = "";
    uint8_t command_buf[16] = "";
    uint16_t ret = 0;
    uint8_t i = 0;
    if(!status)
        command_buf[12] = (uint8_t)(generic_packet.command_tag & 0xFF);

    command_buf[0] = fp_t->start_byte;
    command_buf[1] = generic_packet.packet_type;
    command_buf[2] = 0x0c;
    command_buf[3] = 0;
    command_buf[4] = 0xA0;
    command_buf[7] = 0x02;
    
    ret = crc_update(command_buf,16);
    buf[4] = (uint8_t)(ret & 0xff);
    buf[5] = (uint8_t)((ret & 0xff00) >> 8);
    
    for(i = 0; i < 4; i++)
        buf[i] = command_buf[i];
    for(i = 6; i < 18; i++)
        buf[i] = command_buf[i-2];
    bl_remap_ops.bl_send_data(0, buf, sizeof(buf));
    memset(fp_t,0,sizeof(framing_packet_t));
}

static void write_memory(framing_packet_t *fp_t)
{
    ack_packet();
   
    analysis_app_download_addr(fp_t);
    bl_variate.write_memory_start = app_program.flash_start_addr;
    generic_packet.packet_type = fp_t->packet_type;
    generic_packet.command_tag = fp_t->data[0] & 0xFF;
    generic_response_packet(fp_t,0);
}

static void ack_packet(void)
{
    uint8_t buf[] = {0x5a, 0xa1};
    bl_remap_ops.bl_delayms(5);
    bl_remap_ops.bl_send_data(0, buf, sizeof(buf));
    bl_remap_ops.bl_delayms(5);    
}

uint8_t bl_connect(void)
{
    return bl_variate.app_flag;
}

uint8_t check_app_pc(uint32_t appaddr)
{
    uint32_t *vector_table = (uint32_t *)appaddr;
    uint32_t pc = vector_table[1];
    
    if(pc < bl_remap_ops.app_address || pc > (bl_remap_ops.app_address + 128 * 1024))
        return 0;
    else
        return 1;
}
