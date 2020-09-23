// commands.h
// Tartan Artibeus EXPT board commands header file
//
// Written by Bradley Denby
// Other contributors: None
//
// See the top-level LICENSE file for the license.

// Standard library
#include <stddef.h>   // size_t
#include <stdint.h>   // uint8_t, uint16_t, uint32_t, int16_t

// Macros

//// General
#define CMD_MAX_LEN  258
#define START_BYTE_0 0x22
#define START_BYTE_1 0x69

//// Command opcodes
#define APP_GET_TELEM_OPCODE         0x17
#define APP_GET_TIME_OPCODE          0x13
#define APP_REBOOT_OPCODE            0x12
#define APP_SET_TIME_OPCODE          0x14
#define APP_TELEM_OPCODE             0x18
#define BOOTLOADER_ACK_OPCODE        0x01
#define BOOTLOADER_ERASE_OPCODE      0x0c
#define BOOTLOADER_NACK_OPCODE       0x0f
#define BOOTLOADER_PING_OPCODE       0x00
#define BOOTLOADER_WRITE_PAGE_OPCODE 0x02
#define COMMON_ACK_OPCODE            0x10
#define COMMON_ASCII_OPCODE          0x11
#define COMMON_NACK_OPCODE           0xff

//// Bootloader ACK reasons
#define BOOTLOADER_ACK_REASON_PONG   0x00
#define BOOTLOADER_ACK_REASON_ERASED 0x01

//// Destination IDs
#define LST       0x01
#define LST_RELAY 0x11

// Typedefs
typedef uint16_t cmd_size_t;
typedef uint8_t  cmd_opcode_t;

// Helper functions
void set_start_bytes(char* cmd_o, const cmd_size_t cmd_o_len);
void set_body_len(
 char* cmd_o, const cmd_size_t cmd_o_len, const uint8_t body_len
);
void set_hw_id(char* cmd_o, const cmd_size_t cmd_o_len, const uint16_t hw_id);
void set_msg_id(char* cmd_o, const cmd_size_t cmd_o_len, const uint16_t msg_id);
void set_dest_id(
 char* cmd_o, const cmd_size_t cmd_o_len, const uint8_t dest_id
);
void set_opcode(
 char* cmd_o, const cmd_size_t cmd_o_len, const cmd_opcode_t opcode
);
void set_trailing_zeros(
 char* cmd_o, const cmd_size_t cmd_o_len, const size_t start_index
);

// Command generation functions
void gen_app_get_telem(
 char* cmd_o, const cmd_size_t cmd_o_len, const uint16_t hw_id,
 const uint16_t msg_id, const uint8_t dest_id
);

void gen_app_get_time(
 char* cmd_o, const cmd_size_t cmd_o_len, const uint16_t hw_id,
 const uint16_t msg_id,const uint8_t dest_id
);

void gen_app_reboot(
 char* cmd_o, const cmd_size_t cmd_o_len, const uint16_t hw_id,
 const uint16_t msg_id, const uint8_t dest_id, const uint32_t delay_sec
);

void gen_app_set_time(
 char* cmd_o, const cmd_size_t cmd_o_len, const uint16_t hw_id,
 const uint16_t msg_id, const uint8_t dest_id, const uint32_t seconds,
 const uint32_t nanoseconds
);

void gen_app_telem(
 char* cmd_o, const cmd_size_t cmd_o_len, const uint16_t hw_id,
 const uint16_t msg_id, const uint8_t dest_id, const uint32_t uart_rx_count
);

void gen_bootloader_ack(
 char* cmd_o, const cmd_size_t cmd_o_len, const uint16_t hw_id,
 const uint16_t msg_id, const uint8_t dest_id, const int16_t reason
);

void gen_bootloader_erase(
 char* cmd_o, const cmd_size_t cmd_o_len, const uint16_t hw_id,
 const uint16_t msg_id, const uint8_t dest_id, const int16_t status
);

void gen_bootloader_nack(
 char* cmd_o, const cmd_size_t cmd_o_len, const uint16_t hw_id,
 const uint16_t msg_id, const uint8_t dest_id
);

void gen_bootloader_ping(
 char* cmd_o, const cmd_size_t cmd_o_len, const uint16_t hw_id,
 const uint16_t msg_id, const uint8_t dest_id
);

void gen_bootloader_write_page(
 char* cmd_o, const cmd_size_t cmd_o_len, const uint16_t hw_id,
 const uint16_t msg_id, const uint8_t dest_id, const int16_t page_id,
 const char* page_data, const uint8_t page_data_len
);

void gen_common_ack(
 char* cmd_o, const cmd_size_t cmd_o_len, const uint16_t hw_id,
 const uint16_t msg_id, const uint8_t dest_id
);

void gen_common_ascii(
 char* cmd_o, const cmd_size_t cmd_o_len, const uint16_t hw_id,
 const uint16_t msg_id, const uint8_t dest_id, const char* ascii_data,
 const uint8_t ascii_data_len
);

void gen_common_nack(
 char* cmd_o, const cmd_size_t cmd_o_len, const uint16_t hw_id,
 const uint16_t msg_id, const uint8_t dest_id
);
