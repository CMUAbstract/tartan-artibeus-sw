// commands.c
// Tartan Artibeus EXPT board commands file
//
// Written by Bradley Denby
// Other contributors: None
//
// See the top-level LICENSE file for the license.

// Standard library
#include <stddef.h>   // size_t
#include <stdint.h>   // uint8_t, uint16_t, uint32_t, int16_t

// ta-expt library
#include "commands.h" // ta-expt command macros, typedefs, functions, etc

// Helper functions

void set_start_bytes(char* cmd_o, const cmd_size_t cmd_o_len) {
  if(0<cmd_o_len) {
    cmd_o[0] = START_BYTE_0;
  }
  if(1<cmd_o_len) {
    cmd_o[1] = START_BYTE_1;
  }
}

void set_body_len(
 char* cmd_o, const cmd_size_t cmd_o_len, const uint8_t body_len
) {
  if(2<cmd_o_len) {
    cmd_o[2] = body_len;
  }
}

void set_hw_id(char* cmd_o, const cmd_size_t cmd_o_len, const uint16_t hw_id) {
  if(3<cmd_o_len) { // hw_id least significant byte
    cmd_o[3] = (hw_id) & 0x00ff;
  }
  if(4<cmd_o_len) { // hw_id most significant byte
    cmd_o[4] = (hw_id >> 8) & 0x00ff;
  }
}

void set_msg_id(
 char* cmd_o, const cmd_size_t cmd_o_len, const uint16_t msg_id
) {
  if(5<cmd_o_len) { // msg_id least significant byte
    cmd_o[5] = (msg_id) & 0x00ff;
  }
  if(6<cmd_o_len) { // msg_id most significant byte
    cmd_o[6] = (msg_id >> 8) & 0x00ff;
  }
}

void set_dest_id(
 char* cmd_o, const cmd_size_t cmd_o_len, const uint8_t dest_id
) {
  if(7<cmd_o_len) {
    cmd_o[7] = dest_id;
  }
}

void set_opcode(
 char* cmd_o, const cmd_size_t cmd_o_len, const cmd_opcode_t opcode
) {
  if(8<cmd_o_len) {
    cmd_o[8] = opcode;
  }
}

void set_trailing_zeros(
 char* cmd_o, const cmd_size_t cmd_o_len, const size_t start_index
) {
  for(size_t i=start_index; i<cmd_o_len; i++) {
    cmd_o[i] = 0x00;
  }
}

// Command generation functions

void gen_app_get_telem(
 char* cmd_o, const cmd_size_t cmd_o_len, const uint16_t hw_id,
 const uint16_t msg_id, const uint8_t dest_id
) {
  set_start_bytes(cmd_o, cmd_o_len);
  set_body_len(
   cmd_o, cmd_o_len,
   sizeof(hw_id)+sizeof(msg_id)+sizeof(dest_id)+sizeof(cmd_opcode_t)
  );
  set_hw_id(cmd_o, cmd_o_len, hw_id);
  set_msg_id(cmd_o, cmd_o_len, msg_id);
  set_dest_id(cmd_o, cmd_o_len, dest_id);
  set_opcode(cmd_o, cmd_o_len, APP_GET_TELEM_OPCODE);
  set_trailing_zeros(cmd_o, cmd_o_len, 9);
}

void gen_app_get_time(
 char* cmd_o, const cmd_size_t cmd_o_len, const uint16_t hw_id,
 const uint16_t msg_id, const uint8_t dest_id
) {
  set_start_bytes(cmd_o, cmd_o_len);
  set_body_len(
   cmd_o, cmd_o_len,
   sizeof(hw_id)+sizeof(msg_id)+sizeof(dest_id)+sizeof(cmd_opcode_t)
  );
  set_hw_id(cmd_o, cmd_o_len, hw_id);
  set_msg_id(cmd_o, cmd_o_len, msg_id);
  set_dest_id(cmd_o, cmd_o_len, dest_id);
  set_opcode(cmd_o, cmd_o_len, APP_GET_TIME_OPCODE);
  set_trailing_zeros(cmd_o, cmd_o_len, 9);
}

//// delay_sec is optional; set to zero to omit
void gen_app_reboot(
 char* cmd_o, const cmd_size_t cmd_o_len, const uint16_t hw_id,
 const uint16_t msg_id, const uint8_t dest_id, const uint32_t delay_sec
) {
  set_start_bytes(cmd_o, cmd_o_len);
  if(delay_sec==0) {
    set_body_len(
     cmd_o, cmd_o_len,
     sizeof(hw_id)+sizeof(msg_id)+sizeof(dest_id)+sizeof(cmd_opcode_t)
    );
  } else {
    set_body_len(
     cmd_o, cmd_o_len,
     sizeof(hw_id)+sizeof(msg_id)+sizeof(dest_id)+sizeof(cmd_opcode_t)+
     sizeof(delay_sec)
    );
  }
  set_hw_id(cmd_o, cmd_o_len, hw_id);
  set_msg_id(cmd_o, cmd_o_len, msg_id);
  set_dest_id(cmd_o, cmd_o_len, dest_id);
  set_opcode(cmd_o, cmd_o_len, APP_REBOOT_OPCODE);
  if(delay_sec==0) {
    set_trailing_zeros(cmd_o, cmd_o_len, 9);
  } else {
    if(9<cmd_o_len) { // delay_sec least significant byte
      cmd_o[9] = (delay_sec) & 0x000000ff;
    }
    if(10<cmd_o_len) {
      cmd_o[10] = (delay_sec >> 8) & 0x000000ff;
    }
    if(11<cmd_o_len) {
      cmd_o[11] = (delay_sec >> 16) & 0x000000ff;
    }
    if(12<cmd_o_len) { // delay_sec most significant byte
      cmd_o[12] = (delay_sec >> 24) & 0x000000ff;
    }
    set_trailing_zeros(cmd_o, cmd_o_len, 13);
  }
}

void gen_app_set_time(
 char* cmd_o, const cmd_size_t cmd_o_len, const uint16_t hw_id,
 const uint16_t msg_id, const uint8_t dest_id, const uint32_t seconds,
 const uint32_t nanoseconds
) {
  set_start_bytes(cmd_o, cmd_o_len);
  set_body_len(
   cmd_o, cmd_o_len,
   sizeof(hw_id)+sizeof(msg_id)+sizeof(dest_id)+sizeof(cmd_opcode_t)+
   sizeof(seconds)+sizeof(nanoseconds)
  );
  set_hw_id(cmd_o, cmd_o_len, hw_id);
  set_msg_id(cmd_o, cmd_o_len, msg_id);
  set_dest_id(cmd_o, cmd_o_len, dest_id);
  set_opcode(cmd_o, cmd_o_len, APP_SET_TIME_OPCODE);
  if(9<cmd_o_len) { // seconds least significant byte
    cmd_o[9] = (seconds) & 0x000000ff;
  }
  if(10<cmd_o_len) {
    cmd_o[10] = (seconds >> 8) & 0x000000ff;
  }
  if(11<cmd_o_len) {
    cmd_o[11] = (seconds >> 16) & 0x000000ff;
  }
  if(12<cmd_o_len) { // seconds most significant byte
    cmd_o[12] = (seconds >> 24) & 0x000000ff;
  }
  if(13<cmd_o_len) { // nanoseconds least significant byte
    cmd_o[13] = (nanoseconds) & 0x000000ff;
  }
  if(14<cmd_o_len) {
    cmd_o[14] = (nanoseconds >> 8) & 0x000000ff;
  }
  if(15<cmd_o_len) {
    cmd_o[15] = (nanoseconds >> 16) & 0x000000ff;
  }
  if(16<cmd_o_len) { // nanoseconds most significant byte
    cmd_o[16] = (nanoseconds >> 24) & 0x000000ff;
  }
  set_trailing_zeros(cmd_o, cmd_o_len, 17);
}

void gen_app_telem(
 char* cmd_o, const cmd_size_t cmd_o_len, const uint16_t hw_id,
 const uint16_t msg_id, const uint8_t dest_id, const uint32_t uart_rx_count
) {
  set_start_bytes(cmd_o, cmd_o_len);
  set_body_len(
   cmd_o, cmd_o_len,
   sizeof(hw_id)+sizeof(msg_id)+sizeof(dest_id)+sizeof(cmd_opcode_t)+
   sizeof(uart_rx_count)
  );
  set_hw_id(cmd_o, cmd_o_len, hw_id);
  set_msg_id(cmd_o, cmd_o_len, msg_id);
  set_dest_id(cmd_o, cmd_o_len, dest_id);
  set_opcode(cmd_o, cmd_o_len, APP_TELEM_OPCODE);
  if(9<cmd_o_len) { // uart_rx_count least significant byte
    cmd_o[9] = (uart_rx_count) & 0x000000ff;
  }
  if(10<cmd_o_len) {
    cmd_o[10] = (uart_rx_count >> 8) & 0x000000ff;
  }
  if(11<cmd_o_len) {
    cmd_o[11] = (uart_rx_count >> 16) & 0x000000ff;
  }
  if(12<cmd_o_len) { // uart_rx_count most significant byte
    cmd_o[12] = (uart_rx_count >> 24) & 0x000000ff;
  }
  set_trailing_zeros(cmd_o, cmd_o_len, 13);
}

//// reason is optional; set to less than zero to omit
void gen_bootloader_ack(
 char* cmd_o, const cmd_size_t cmd_o_len, const uint16_t hw_id,
 const uint16_t msg_id, const uint8_t dest_id, const int16_t reason
) {
  set_start_bytes(cmd_o, cmd_o_len);
  if(reason<0) {
    set_body_len(
     cmd_o, cmd_o_len,
     sizeof(hw_id)+sizeof(msg_id)+sizeof(dest_id)+sizeof(cmd_opcode_t)
    );
  } else {
    set_body_len(
     cmd_o, cmd_o_len,
     sizeof(hw_id)+sizeof(msg_id)+sizeof(dest_id)+sizeof(cmd_opcode_t)+
     sizeof(reason)
    );
  }
  set_hw_id(cmd_o, cmd_o_len, hw_id);
  set_msg_id(cmd_o, cmd_o_len, msg_id);
  set_dest_id(cmd_o, cmd_o_len, dest_id);
  set_opcode(cmd_o, cmd_o_len, BOOTLOADER_ACK_OPCODE);
  if(reason<0) {
    set_trailing_zeros(cmd_o, cmd_o_len, 9);
  } else {
    if(9<cmd_o_len) { // reason least significant byte
      cmd_o[9] = (reason) & 0x00ff;
    }
    if(10<cmd_o_len) { // reason most significant byte
      cmd_o[10] = (reason >> 8) & 0x00ff;
    }
    set_trailing_zeros(cmd_o, cmd_o_len, 11);
  }
}

//// status is optional; set to less than zero to omit
void gen_bootloader_erase(
 char* cmd_o, const cmd_size_t cmd_o_len, const uint16_t hw_id,
 const uint16_t msg_id, const uint8_t dest_id, const int16_t status
) {
  set_start_bytes(cmd_o, cmd_o_len);
  if(status<0) {
    set_body_len(
     cmd_o, cmd_o_len,
     sizeof(hw_id)+sizeof(msg_id)+sizeof(dest_id)+sizeof(cmd_opcode_t)
    );
  } else {
    set_body_len(
     cmd_o, cmd_o_len,
     sizeof(hw_id)+sizeof(msg_id)+sizeof(dest_id)+sizeof(cmd_opcode_t)+
     sizeof(status)
    );
  }
  set_hw_id(cmd_o, cmd_o_len, hw_id);
  set_msg_id(cmd_o, cmd_o_len, msg_id);
  set_dest_id(cmd_o, cmd_o_len, dest_id);
  set_opcode(cmd_o, cmd_o_len, BOOTLOADER_ERASE_OPCODE);
  if(status<0) {
    set_trailing_zeros(cmd_o, cmd_o_len, 9);
  } else {
    if(9<cmd_o_len) { // status least significant byte
      cmd_o[9] = (status) & 0x00ff;
    }
    if(10<cmd_o_len) { // status most significant byte
      cmd_o[10] = (status >> 8) & 0x00ff;
    }
    set_trailing_zeros(cmd_o, cmd_o_len, 11);
  }
}

void gen_bootloader_nack(
 char* cmd_o, const cmd_size_t cmd_o_len, const uint16_t hw_id,
 const uint16_t msg_id, const uint8_t dest_id
) {
  set_start_bytes(cmd_o, cmd_o_len);
  set_body_len(
   cmd_o, cmd_o_len,
   sizeof(hw_id)+sizeof(msg_id)+sizeof(dest_id)+sizeof(cmd_opcode_t)
  );
  set_hw_id(cmd_o, cmd_o_len, hw_id);
  set_msg_id(cmd_o, cmd_o_len, msg_id);
  set_dest_id(cmd_o, cmd_o_len, dest_id);
  set_opcode(cmd_o, cmd_o_len, BOOTLOADER_NACK_OPCODE);
  set_trailing_zeros(cmd_o, cmd_o_len, 9);
}

void gen_bootloader_ping(
 char* cmd_o, const cmd_size_t cmd_o_len, const uint16_t hw_id,
 const uint16_t msg_id, const uint8_t dest_id
) {
  set_start_bytes(cmd_o, cmd_o_len);
  set_body_len(
   cmd_o, cmd_o_len,
   sizeof(hw_id)+sizeof(msg_id)+sizeof(dest_id)+sizeof(cmd_opcode_t)
  );
  set_hw_id(cmd_o, cmd_o_len, hw_id);
  set_msg_id(cmd_o, cmd_o_len, msg_id);
  set_dest_id(cmd_o, cmd_o_len, dest_id);
  set_opcode(cmd_o, cmd_o_len, BOOTLOADER_PING_OPCODE);
  set_trailing_zeros(cmd_o, cmd_o_len, 9);
}

//// page_data is optional; set page_id to less than zero or page_data_len to
//// zero to omit page_data; if page_id is set to less than zero it will be
//// clipped to zero
void gen_bootloader_write_page(
 char* cmd_o, const cmd_size_t cmd_o_len, const uint16_t hw_id,
 const uint16_t msg_id, const uint8_t dest_id, const int16_t page_id,
 const char* page_data, const uint8_t page_data_len
) {
  set_start_bytes(cmd_o, cmd_o_len);
  if(page_id<0 || page_data_len==0) {
    set_body_len(
     cmd_o, cmd_o_len,
     sizeof(hw_id)+sizeof(msg_id)+sizeof(dest_id)+sizeof(cmd_opcode_t)+
     sizeof(page_id)
    );
  } else {
    set_body_len(
     cmd_o, cmd_o_len,
     sizeof(hw_id)+sizeof(msg_id)+sizeof(dest_id)+sizeof(cmd_opcode_t)+
     sizeof(page_id)+page_data_len
    );
  }
  set_hw_id(cmd_o, cmd_o_len, hw_id);
  set_msg_id(cmd_o, cmd_o_len, msg_id);
  set_dest_id(cmd_o, cmd_o_len, dest_id);
  set_opcode(cmd_o, cmd_o_len, BOOTLOADER_WRITE_PAGE_OPCODE);
  if(page_id<0 || page_data_len==0) {
    if(9<cmd_o_len) { // page_id least significant byte
      cmd_o[9] = 0x00;
    }
    if(10<cmd_o_len) { // page_id most significant byte
      cmd_o[10] = 0x00;
    }
    set_trailing_zeros(cmd_o, cmd_o_len, 11);
  } else {
    if(9<cmd_o_len) { // page_id least significant byte
      cmd_o[9] = (page_id) & 0x00ff;
    }
    if(10<cmd_o_len) { // page_id most significant byte
      cmd_o[10] = (page_id >> 8) & 0x00ff;
    }
    size_t cmd_o_index = 11;
    for(size_t i=0; i<page_data_len && cmd_o_index<CMD_MAX_LEN; i++) {
      cmd_o[cmd_o_index] = page_data[i];
      cmd_o_index++;
    }
    set_trailing_zeros(cmd_o, cmd_o_len, cmd_o_index);
  }
}

void gen_common_ack(
 char* cmd_o, const cmd_size_t cmd_o_len, const uint16_t hw_id,
 const uint16_t msg_id, const uint8_t dest_id
) {
  set_start_bytes(cmd_o, cmd_o_len);
  set_body_len(
   cmd_o, cmd_o_len,
   sizeof(hw_id)+sizeof(msg_id)+sizeof(dest_id)+sizeof(cmd_opcode_t)
  );
  set_hw_id(cmd_o, cmd_o_len, hw_id);
  set_msg_id(cmd_o, cmd_o_len, msg_id);
  set_dest_id(cmd_o, cmd_o_len, dest_id);
  set_opcode(cmd_o, cmd_o_len, COMMON_ACK_OPCODE);
  set_trailing_zeros(cmd_o, cmd_o_len, 9);
}

void gen_common_ascii(
 char* cmd_o, const cmd_size_t cmd_o_len, const uint16_t hw_id,
 const uint16_t msg_id, const uint8_t dest_id, const char* ascii_data,
 const uint8_t ascii_data_len
) {
  set_start_bytes(cmd_o, cmd_o_len);
  set_body_len(
   cmd_o, cmd_o_len,
   sizeof(hw_id)+sizeof(msg_id)+sizeof(dest_id)+sizeof(cmd_opcode_t)+
   ascii_data_len
  );
  set_hw_id(cmd_o, cmd_o_len, hw_id);
  set_msg_id(cmd_o, cmd_o_len, msg_id);
  set_dest_id(cmd_o, cmd_o_len, dest_id);
  set_opcode(cmd_o, cmd_o_len, COMMON_ASCII_OPCODE);
  size_t cmd_o_index = 9;
  for(size_t i=0; i<ascii_data_len && cmd_o_index<CMD_MAX_LEN; i++) {
    cmd_o[cmd_o_index] = ascii_data[i];
    cmd_o_index++;
  }
  set_trailing_zeros(cmd_o, cmd_o_len, cmd_o_index);
}

void gen_common_nack(
 char* cmd_o, const cmd_size_t cmd_o_len, const uint16_t hw_id,
 const uint16_t msg_id, const uint8_t dest_id
) {
  set_start_bytes(cmd_o, cmd_o_len);
  set_body_len(
   cmd_o, cmd_o_len,
   sizeof(hw_id)+sizeof(msg_id)+sizeof(dest_id)+sizeof(cmd_opcode_t)
  );
  set_hw_id(cmd_o, cmd_o_len, hw_id);
  set_msg_id(cmd_o, cmd_o_len, msg_id);
  set_dest_id(cmd_o, cmd_o_len, dest_id);
  set_opcode(cmd_o, cmd_o_len, COMMON_NACK_OPCODE);
  set_trailing_zeros(cmd_o, cmd_o_len, 9);
}
