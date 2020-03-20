#ifndef __BL_TEST_H
#define __BL_TEST_H
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "common.h"
#define PINT_RESPONSE_PACKET_P 0x50
#define BOOTLOADER_VERSION 0x4b010500
#define LENGTH 256
#define BL_TRACE printf


typedef struct bl_test_func_ops {
    uint32_t flash_size;
    uint32_t flash_sector_size;
    uint32_t packet_size;
    uint32_t device_id;
    uint32_t app_address;
    void (*reset_mcu)(void);
    void (*bl_send_data)(uint8_t instance, uint8_t *buf, uint32_t length);
    void (*bl_delayms)(uint32_t num);
    void (*bl_write_memory)(uint32_t addr, uint16_t *buf, uint16_t len);
    uint8_t (*bl_flash_erase_region)(uint32_t addr);
}bl_test_func_ops_t;

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

void bl_test_init(bl_test_func_ops_t *bl_remap);
void JumpToImage(uint32_t addr);
uint8_t check_app_pc(uint32_t appaddr);
uint16_t crc_update(const uint8_t *src, uint32_t LengthInBytes);
uint8_t bl_connect(void);

void analysis_packet(framing_packet_t *fp_t, uint8_t data);


#endif
