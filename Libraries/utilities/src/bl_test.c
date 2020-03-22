#include "bl_test.h"

static uint16_t count = 0;
static uint16_t length = 0;
static uint8_t status = KStatus_Start;
uint8_t app_flag;
uint8_t write_memory_flag;
uint32_t write_memory_start;

generic_response_packet_t generic_packet;
app_program_location_t app_program;

bl_test_func_ops_t bl_remap_ops;


uint16_t check_crc(framing_packet_t *fp_t, const uint16_t length);
void flash_erase_region(framing_packet_t *fp_t);
void analysis_command_tag(framing_packet_t *fp_t);
void property_tag(framing_packet_t *fp_t);
void command_response_packet(framing_packet_t *fp_t, uint32_t value);
void generic_response_packet(framing_packet_t *fp_t, uint8_t status);
void write_memory(framing_packet_t *fp_t);
void recv_data(framing_packet_t *fp_t);
void reset_mcu(framing_packet_t *fp_t);
void analysis_app_download_addr(framing_packet_t *fp_t);
void ping_response_packet(void);
void ack_packet(void);

void bl_test_init(bl_test_func_ops_t *bl_remap)
{
    memcpy(&bl_remap_ops,bl_remap, sizeof(bl_remap_ops));
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
                app_flag = 1;
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
       
            if(length <= bl_remap_ops.packet_size)
            {
                status = KStatus_CrcLow;
            }
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
                check_crc(fp_t, length);
            }
            break;
        default:
            status = KStatus_Start;
    }
}

uint16_t check_crc(framing_packet_t *fp_t, const uint16_t length)
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
                recv_data(fp_t);
                break;
            default:
                BL_TRACE("fp_t->packet_type = [%X]\r\n", fp_t->packet_type);
        }
    }
    else
    {
        printf("%d", length);
        printf("%X", fp_t->crc_low);
        printf("%X", fp_t->crc_high);
        printf("%X", crc_value);
        printf("%X", ret);
    }
    return ret;
}

void analysis_command_tag(framing_packet_t *fp_t)
{
    switch(fp_t->data[0])
        {
            case 0x02:          //FlashEraseRegion
                flash_erase_region(fp_t);
                break;
            case 0x04:          //WriteMemory
                write_memory(fp_t);
                break;
//            case 0x05:          //FillMemory
//                fill_memory();
//                break;
//            case 0x06:          //FlashSecurityDisable
//                flash_security_disable();
//                break;
            case 0x07:          //GetProperty
                property_tag(fp_t);
                break;

//            case 0x09:
//            case 0x10:
            case 0x0B:          //ResetMcu
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

void reset_mcu(framing_packet_t *fp_t)
{
    ack_packet();
    generic_packet.packet_type = kFramingPacketType_Command;
    generic_packet.command_tag = 0x0B;
    generic_response_packet(fp_t, 0);
    bl_remap_ops.reset_mcu();
    while(1);
}

void property_tag(framing_packet_t *fp_t)
{
    switch(fp_t->data[4])
    {
        case 0x01:
            //调用发送bootloader版本号的函数
            command_response_packet(fp_t, BOOTLOADER_VERSION);
            break;
//        case 0x02:
//        case 0x03:
        case 0x04:
            command_response_packet(fp_t, bl_remap_ops.flash_size);
            break;
        case 0x05:
            command_response_packet(fp_t, bl_remap_ops.flash_sector_size);
            break;
//        case 0x06:
//        case 0x07:
//        case 0x0A:
        case 0x0B:
            command_response_packet(fp_t, bl_remap_ops.packet_size);
            break;
        case 0x10:
            command_response_packet(fp_t, bl_remap_ops.device_id);
            break;
        default:
            BL_TRACE("fp_t->data[4] = [%X]\r\n", fp_t->data[4]);
    }        
}

void command_response_packet(framing_packet_t *fp_t, uint32_t value)
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


void ping_response_packet(void)
{
    uint8_t src[10] = "";
    uint16_t ret = 0;
    
    src[0] = kFramingPacketType_StartByte;
    src[1] = kFramingPacketType_PingResponse;
    src[3] = 0x02;
    src[4] = 0x01;
    src[5] = PINT_RESPONSE_PACKET_P;

    
    ret = crc_update(src, 8);
    
    src[8] = ret & 0x00ff;
    src[9] = (ret & 0xff00) >> 8;
    
    bl_remap_ops.bl_delayms(1);
    
    bl_remap_ops.bl_send_data(0,src,10);
}

void recv_data(framing_packet_t *fp_t)
{
    uint16_t length = fp_t->len_low | (uint16_t)fp_t->len_high << 8;

    if(write_memory_flag && write_memory_start >= bl_remap_ops.app_address)
    {
        bl_remap_ops.bl_write_memory(write_memory_start, (uint16_t *)fp_t->data, length / 2);
        write_memory_start += length;
    }
    
    ack_packet();
    
    if(write_memory_start == app_program.flash_byte_count + app_program.flash_start_addr)
    {
        write_memory_start = 0;
   
        generic_packet.packet_type = kFramingPacketType_Command;
        generic_packet.command_tag = 0x04;
        generic_response_packet(fp_t,0);
    }
}

void flash_erase_region(framing_packet_t *fp_t)
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

    write_memory_flag = 1;

    generic_packet.packet_type = fp_t->packet_type;
    generic_packet.command_tag = fp_t->data[0] & 0xFF;
    generic_response_packet(fp_t, 0);
}

void analysis_app_download_addr(framing_packet_t *fp_t)
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

void generic_response_packet(framing_packet_t *fp_t, uint8_t status)
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

void write_memory(framing_packet_t *fp_t)
{
    ack_packet();
    if(write_memory_flag)
    {
        analysis_app_download_addr(fp_t);
        write_memory_start = app_program.flash_start_addr;
        generic_packet.packet_type = fp_t->packet_type;
        generic_packet.command_tag = fp_t->data[0] & 0xFF;
        generic_response_packet(fp_t,0);
    }
}

void ack_packet(void)
{
    uint8_t buf[] = {0x5a, 0xa1};
    bl_remap_ops.bl_delayms(5);
    bl_remap_ops.bl_send_data(0, buf, sizeof(buf));
    bl_remap_ops.bl_delayms(5);    
}

uint8_t bl_connect(void)
{
    return app_flag;
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

void JumpToImage(uint32_t addr)
{
    uint32_t *vector_table = (uint32_t *)addr;
    uint32_t sp = vector_table[0];
    uint32_t pc = vector_table[1];
    typedef void(*applcation_entry)(void);
    applcation_entry app_address = (applcation_entry) pc;

    __set_MSP(sp);
    __set_PSP(sp);
    
    SCB->VTOR = addr;
    
    app_address();
}
