// taolst_protocol.c
// Tartan Artibeus OLST serial communication protocol implementation file
//
// Written by Bradley Denby
// Other contributors: None
//
// See the top-level LICENSE file for the license.

// Standard library
#include <stddef.h>                 // size_t
#include <stdint.h>                 // uint8_t, uint32_t, uint64_t

// libopencm3 library
#include <libopencm3/stm32/flash.h> // flash erase and write
#include <libopencm3/stm32/rtc.h>

// ta-expt library
#include <bootloader.h>             // Bootloader macros
#include <taolst_protocol.h>        // Header file

// Variables
extern int app_jump_pending; // Used in bootloader main to signal jump to app

// Helper functions

//// Clears rx_cmd_buff data and resets state and indices
void clear_rx_cmd_buff(rx_cmd_buff_t* rx_cmd_buff_o) {
  rx_cmd_buff_o->state = RX_CMD_BUFF_STATE_START_BYTE_0;
  rx_cmd_buff_o->start_index = 0;
  rx_cmd_buff_o->end_index = 0;
  for(size_t i=0; i<rx_cmd_buff_o->size; i++) {
    rx_cmd_buff_o->data[i] = ((uint8_t)0x00);
  }
}

//// Clears tx_cmd_buff data and resets state and indices
void clear_tx_cmd_buff(tx_cmd_buff_t* tx_cmd_buff_o) {
  tx_cmd_buff_o->empty = 1;
  tx_cmd_buff_o->start_index = 0;
  tx_cmd_buff_o->end_index = 0;
  for(size_t i=0; i<tx_cmd_buff_o->size; i++) {
    tx_cmd_buff_o->data[i] = ((uint8_t)0x00);
  }
}

int app_subroutine(int32_t seconds, int32_t nanoseconds) {
  float days = ((float) seconds) / 86400.0f;
  int32_t JD = (int32_t) (2451545.0f + days + 0.5f);
  int32_t I, J, K, L, N;
  L = JD + 68569;
  N = 4 * L / 146097;
  L = L - (146097 * N + 3) / 4;
  I = 4000 * (L + 1) / 1461001;
  L = L - 1461 * I / 4 + 31;
  J = 80 * L / 2447;
  K = L - 2447 * J / 80;
  L = J / 11;
  J = J + 2 - 12 * L;
  I = 100 * (N - 49) + I + L;
  
  rtc_unlock();
  //set RTC_DR_YT[3:0], Date Register bits [23:20] 
  //and RTC_DR_YU[3:0], Date Register bits [19:16]
  rtc_calendar_set_year((uint8_t) I);
  
  //set RTC_DR_MT,      Date Register bit  [12] 
  //and RTC_DR_MU[3:0], Date Register bits [11:8]
  rtc_calendar_set_month((uint8_t) J);

  //set RTC_DR_DT[1:0], Date Register bits [5:4] 
  //and RTC_DR_DU[3:0], Date Register bits [3:0]
  rtc_calendar_set_day((uint8_t) K);
  
  int32_t newJD = K - 32075 + 1461*(I + 4800 + (J - 14)/12)/4
                   + 367*(J - 2 - (J - 14)/12*12)/12 - 3
                   *((I + 4900 + (J - 14)/12)/100)/4;
  int32_t remainder = ((2451545.0f + days) - ((float) newJD - 0.5f))*86400.0f; //seconds
  int32_t hour = remainder / 3600;
  int32_t min = (remainder % 3600) / 60;
  int32_t sec = (remainder % 3600) % 60;
  
  //set RTC_TR_PM,       Time Register bit  [22]
  //and RTC_TR_HT[3:0],  Time Register bits [21:20]
  //and RTC_TR_HU[3:0],  Time Register bits [19:16]
  //and RTC_TR_MNT[3:0], Time Register bits [14:12]
  //and RTC_TR_MNU[3:0], Time Register bits [11:8]
  //and RTC_TR_ST[3:0],  Time Register bits [6:4]
  //and RTC_TR_SU[3:0],  Time Register bits [3:0]
  rtc_time_set_time((uint8_t) hour, (uint8_t) min, (uint8_t) sec, false);
  rtc_lock();
  
  //trivial return statement, currently has no way to set nanoseconds
  return nanoseconds * 0 + 1;
}

int bootloader_erase(rx_cmd_buff_t* rx_cmd_buff) {
  if(
   rx_cmd_buff->state==RX_CMD_BUFF_STATE_COMPLETE &&
   rx_cmd_buff->data[OPCODE_INDEX]==BOOTLOADER_ERASE_OPCODE
  ) {
    return 1;
  }
  else return 0; 
}

int bootloader_ping(rx_cmd_buff_t* rx_cmd_buff) {
  if(
   rx_cmd_buff->state==RX_CMD_BUFF_STATE_COMPLETE &&
   rx_cmd_buff->data[OPCODE_INDEX]==BOOTLOADER_PING_OPCODE
  ) {
    return 1;
  }
  else return 0; 
}

// Command functions
int parse_ascii_string(rx_cmd_buff_t* rx_cmd_buff) {
  size_t first = rx_cmd_buff->start_index;
  return first * 0;
}
//// Given a well-formed BOOTLOADER_WRITE_PAGE command, write data to flash
int bootloader_write_data(rx_cmd_buff_t* rx_cmd_buff) {
  if(
   rx_cmd_buff->state==RX_CMD_BUFF_STATE_COMPLETE &&
   rx_cmd_buff->data[OPCODE_INDEX]==BOOTLOADER_WRITE_PAGE_OPCODE
  ) {
    flash_unlock();
    uint32_t subpage_id = (uint32_t)(rx_cmd_buff->data[DATA_START_INDEX]);
    // subpage_id==0x00 writes to APP_ADDR==0x08008000 i.e. start of page 16
    // So subpage_id==0x10 writes to addr 0x08008800 i.e. start of page 17 etc
    // Need to erase page once before writing inside of it
    if((subpage_id*BYTES_PER_CMD)%BYTES_PER_PAGE==0) {
      flash_erase_page(16+(subpage_id*BYTES_PER_CMD)/BYTES_PER_PAGE);
      flash_clear_status_flags();
    }
    // write data
    uint32_t start_addr = APP_ADDR+subpage_id*BYTES_PER_CMD;
    for(size_t i=0; i<BYTES_PER_CMD; i+=8) {
      uint64_t dword = *(uint64_t*)((rx_cmd_buff->data)+DATA_START_INDEX+1+i);
      flash_wait_for_last_operation();
      FLASH_CR |= FLASH_CR_PG;
      MMIO32(i+start_addr)   = (uint32_t)(dword);
      MMIO32(i+start_addr+4) = (uint32_t)(dword >> 32);
      flash_wait_for_last_operation();
      FLASH_CR &= ~FLASH_CR_PG;
      flash_clear_status_flags();
    }
    flash_lock();
    return 1;
  } else {
    return 0;
  }
}

/*uint8_t *common_get_char(rx_cmd_buff_t* rx_cmd_buff) {
  int len = rx_cmd_buff->data[MSG_LEN_INDEX] - 0x06;
  uint8_t res[len];
  for (int i = 0; i < len; i++) {
    res[i] = rx_cmd_buff->data[i+0x09];
  }
  return res;
}
*/
// Protocol functions

//// Attempts to push byte to end of rx_cmd_buff
void push_rx_cmd_buff(rx_cmd_buff_t* rx_cmd_buff_o, uint8_t b) {
  switch(rx_cmd_buff_o->state) {
    case RX_CMD_BUFF_STATE_START_BYTE_0:
      if(b==START_BYTE_0) {
        rx_cmd_buff_o->data[START_BYTE_0_INDEX] = b;
        rx_cmd_buff_o->state = RX_CMD_BUFF_STATE_START_BYTE_1;
      }
      break;
    case RX_CMD_BUFF_STATE_START_BYTE_1:
      if(b==START_BYTE_1) {
        rx_cmd_buff_o->data[START_BYTE_1_INDEX] = b;
        rx_cmd_buff_o->state = RX_CMD_BUFF_STATE_MSG_LEN;
      } else {
        clear_rx_cmd_buff(rx_cmd_buff_o);
      }
      break;
    case RX_CMD_BUFF_STATE_MSG_LEN:
      if(((uint8_t)0x06)<=b /*&& b<=((uint8_t)0xff)*/) {
        rx_cmd_buff_o->data[MSG_LEN_INDEX] = b;
        rx_cmd_buff_o->start_index = ((uint8_t)0x09);
        rx_cmd_buff_o->end_index = (b+((uint8_t)0x03));
        rx_cmd_buff_o->state = RX_CMD_BUFF_STATE_HWID_LSB;
      } else {
        clear_rx_cmd_buff(rx_cmd_buff_o);
      }
      break;
    case RX_CMD_BUFF_STATE_HWID_LSB:
      rx_cmd_buff_o->data[HWID_LSB_INDEX] = b;
      rx_cmd_buff_o->state = RX_CMD_BUFF_STATE_HWID_MSB;
      break;
    case RX_CMD_BUFF_STATE_HWID_MSB:
      rx_cmd_buff_o->data[HWID_MSB_INDEX] = b;
      rx_cmd_buff_o->state = RX_CMD_BUFF_STATE_MSG_ID_LSB;
      break;
    case RX_CMD_BUFF_STATE_MSG_ID_LSB:
      rx_cmd_buff_o->data[MSG_ID_LSB_INDEX] = b;
      rx_cmd_buff_o->state = RX_CMD_BUFF_STATE_MSG_ID_MSB;
      break;
    case RX_CMD_BUFF_STATE_MSG_ID_MSB:
      rx_cmd_buff_o->data[MSG_ID_MSB_INDEX] = b;
      rx_cmd_buff_o->state = RX_CMD_BUFF_STATE_DEST_ID;
      break;
    case RX_CMD_BUFF_STATE_DEST_ID:
      rx_cmd_buff_o->data[DEST_ID_INDEX] = b;
      rx_cmd_buff_o->state = RX_CMD_BUFF_STATE_OPCODE;
      break;
    case RX_CMD_BUFF_STATE_OPCODE: // no check for valid opcodes (too much)
      rx_cmd_buff_o->data[OPCODE_INDEX] = b;
      if(rx_cmd_buff_o->start_index<rx_cmd_buff_o->end_index) {
        rx_cmd_buff_o->state = RX_CMD_BUFF_STATE_DATA;
      } else {
        rx_cmd_buff_o->state = RX_CMD_BUFF_STATE_COMPLETE;
      }
      break;
    case RX_CMD_BUFF_STATE_DATA:
      if(rx_cmd_buff_o->start_index<rx_cmd_buff_o->end_index) {
        rx_cmd_buff_o->data[rx_cmd_buff_o->start_index] = b;
        rx_cmd_buff_o->start_index += 1;
      }
      // Must move to COMPLETE state immediately if b is the last byte
      if(rx_cmd_buff_o->start_index==rx_cmd_buff_o->end_index) {
        rx_cmd_buff_o->state = RX_CMD_BUFF_STATE_COMPLETE;
      }
      break;
    case RX_CMD_BUFF_STATE_COMPLETE:
      // If rx_cmd_buff_t holds a complete command, do nothing with new byte b
      break;
    default:
      // Do nothing by default
      break;
  }
}

//// Attempts to clear rx_cmd_buff and populate tx_cmd_buff with reply
void write_reply(rx_cmd_buff_t* rx_cmd_buff_o, tx_cmd_buff_t* tx_cmd_buff_o) {
  if(
   rx_cmd_buff_o->state==RX_CMD_BUFF_STATE_COMPLETE &&
   tx_cmd_buff_o->empty
  ) {
    tx_cmd_buff_o->data[START_BYTE_0_INDEX] = START_BYTE_0;
    tx_cmd_buff_o->data[START_BYTE_1_INDEX] = START_BYTE_1;
    tx_cmd_buff_o->data[HWID_LSB_INDEX] = rx_cmd_buff_o->data[HWID_LSB_INDEX];
    tx_cmd_buff_o->data[HWID_MSB_INDEX] = rx_cmd_buff_o->data[HWID_MSB_INDEX];
    tx_cmd_buff_o->data[MSG_ID_LSB_INDEX] =
     rx_cmd_buff_o->data[MSG_ID_LSB_INDEX];
    tx_cmd_buff_o->data[MSG_ID_MSB_INDEX] =
     rx_cmd_buff_o->data[MSG_ID_MSB_INDEX];
    //tx_cmd_buff_o->data[DEST_ID_INDEX] = LST_RELAY;
    tx_cmd_buff_o->data[DEST_ID_INDEX] = LST;
    switch(rx_cmd_buff_o->data[OPCODE_INDEX]) {
      case APP_GET_TELEM_OPCODE:
        tx_cmd_buff_o->data[MSG_LEN_INDEX] = ((uint8_t)0x06);
        tx_cmd_buff_o->data[OPCODE_INDEX] = COMMON_NACK_OPCODE;
        break;
      case APP_GET_TIME_OPCODE:
        break;
      case APP_REBOOT_OPCODE:
        break;
      case APP_SET_TIME_OPCODE:
        ;
        //seconds
        uint32_t SecByte1 = (uint32_t) rx_cmd_buff_o->data[DATA_START_INDEX+3]; //Sec MSB
        uint32_t SecByte2 = (uint32_t) rx_cmd_buff_o->data[DATA_START_INDEX+2];
        uint32_t SecByte3 = (uint32_t) rx_cmd_buff_o->data[DATA_START_INDEX+1];
        uint32_t SecByte4 = (uint32_t) rx_cmd_buff_o->data[DATA_START_INDEX];   //Sec LSB
        //nanoseconds
        uint32_t NsByte1  = (uint32_t) rx_cmd_buff_o->data[DATA_START_INDEX+7]; //Ns MSB
        uint32_t NsByte2  = (uint32_t) rx_cmd_buff_o->data[DATA_START_INDEX+6];
        uint32_t NsByte3  = (uint32_t) rx_cmd_buff_o->data[DATA_START_INDEX+5];
        uint32_t NsByte4  = (uint32_t) rx_cmd_buff_o->data[DATA_START_INDEX+4]; //Ns LSB
        //concatenation
        uint32_t seconds = SecByte1 << 24 | SecByte2 << 16 | 
                           SecByte3 << 8  | SecByte4;
        uint32_t nanoseconds = NsByte1 << 24 | NsByte2 << 16 | 
                               NsByte3 << 8  | NsByte4;
        app_subroutine((int32_t) seconds, (int32_t) nanoseconds);
        tx_cmd_buff_o->data[MSG_LEN_INDEX] = ((uint8_t)0x06);
        tx_cmd_buff_o->data[OPCODE_INDEX] = COMMON_ACK_OPCODE;
        break;
      case APP_TELEM_OPCODE:
        tx_cmd_buff_o->data[MSG_LEN_INDEX] = ((uint8_t)0x06);
        tx_cmd_buff_o->data[OPCODE_INDEX] = COMMON_NACK_OPCODE;
        break;
      case BOOTLOADER_ACK_OPCODE:
        tx_cmd_buff_o->data[MSG_LEN_INDEX] = ((uint8_t)0x06);
        tx_cmd_buff_o->data[OPCODE_INDEX] = COMMON_NACK_OPCODE;
        break;
      case BOOTLOADER_ERASE_OPCODE:
        ; // empty statement to avoid weird C thing about vars in switch case
        int success_erase = 0;
        success_erase = bootloader_erase(rx_cmd_buff_o);
        if(success_erase) {
          tx_cmd_buff_o->data[MSG_LEN_INDEX] = ((uint8_t)0x07);
          tx_cmd_buff_o->data[OPCODE_INDEX] = BOOTLOADER_ACK_OPCODE;
          tx_cmd_buff_o->data[DATA_START_INDEX] = BOOTLOADER_ACK_REASON_ERASED; //ERASED
        } else {
          tx_cmd_buff_o->data[MSG_LEN_INDEX] = ((uint8_t)0x06);
          tx_cmd_buff_o->data[OPCODE_INDEX] = COMMON_NACK_OPCODE;
        }
        break;
      case BOOTLOADER_NACK_OPCODE:
        tx_cmd_buff_o->data[MSG_LEN_INDEX] = ((uint8_t)0x06);
        tx_cmd_buff_o->data[OPCODE_INDEX] = COMMON_NACK_OPCODE;
        break;
      case BOOTLOADER_PING_OPCODE:
        ; // empty statement to avoid weird C thing about vars in switch case
        int success_ping = 0;
        success_ping = bootloader_ping(rx_cmd_buff_o);
        if(success_ping) {
          tx_cmd_buff_o->data[MSG_LEN_INDEX] = ((uint8_t)0x07);
          tx_cmd_buff_o->data[OPCODE_INDEX] = BOOTLOADER_ACK_OPCODE;
          tx_cmd_buff_o->data[DATA_START_INDEX] = BOOTLOADER_ACK_REASON_PONG; //PONG
        } else {
          tx_cmd_buff_o->data[MSG_LEN_INDEX] = ((uint8_t)0x06);
          tx_cmd_buff_o->data[OPCODE_INDEX] = COMMON_NACK_OPCODE;
        }
        break;
      case BOOTLOADER_WRITE_PAGE_OPCODE:
        ; // empty statement to avoid weird C thing about vars in switch case
        int success = 0;
        success = bootloader_write_data(rx_cmd_buff_o);
        if(success) {
          tx_cmd_buff_o->data[MSG_LEN_INDEX] = ((uint8_t)0x07);
          tx_cmd_buff_o->data[OPCODE_INDEX] = BOOTLOADER_ACK_OPCODE;
          tx_cmd_buff_o->data[DATA_START_INDEX] =
           rx_cmd_buff_o->data[DATA_START_INDEX];
        } else {
          tx_cmd_buff_o->data[MSG_LEN_INDEX] = ((uint8_t)0x06);
          tx_cmd_buff_o->data[OPCODE_INDEX] = BOOTLOADER_NACK_OPCODE;
        }
        break;
      case BOOTLOADER_JUMP_OPCODE:
        app_jump_pending = 1;
        tx_cmd_buff_o->data[MSG_LEN_INDEX] = ((uint8_t)0x07);
        tx_cmd_buff_o->data[OPCODE_INDEX] = BOOTLOADER_ACK_OPCODE;
        tx_cmd_buff_o->data[DATA_START_INDEX] = BOOTLOADER_ACK_REASON_JUMP;
        break;
      case COMMON_ACK_OPCODE:
        tx_cmd_buff_o->data[MSG_LEN_INDEX] = ((uint8_t)0x06);
        tx_cmd_buff_o->data[OPCODE_INDEX] = COMMON_ACK_OPCODE;
        break;
      case COMMON_ASCII_OPCODE:
        ;
        int success_ascii = 0;
        success_ascii = parse_ascii_string(rx_cmd_buff_o);
        tx_cmd_buff_o->data[MSG_LEN_INDEX] = ((uint8_t)0x06);
        tx_cmd_buff_o->data[OPCODE_INDEX] = COMMON_NACK_OPCODE;
        if (success_ascii) {}
        else {}
        //don't know if something like this is necessary
        break;
      case COMMON_NACK_OPCODE:
        tx_cmd_buff_o->data[MSG_LEN_INDEX] = ((uint8_t)0x06);
        tx_cmd_buff_o->data[OPCODE_INDEX] = COMMON_NACK_OPCODE;
        break;
      default:
        break;
    }
    tx_cmd_buff_o->end_index =
     (tx_cmd_buff_o->data[MSG_LEN_INDEX]+((uint8_t)0x03));
    tx_cmd_buff_o->empty = 0;
    clear_rx_cmd_buff(rx_cmd_buff_o);
  }
}

//// Attempts to pop byte from beginning of tx_cmd_buff
uint8_t pop_tx_cmd_buff(tx_cmd_buff_t* tx_cmd_buff_o) {
  uint8_t b = 0;
  if(
   !tx_cmd_buff_o->empty &&
   tx_cmd_buff_o->start_index<tx_cmd_buff_o->end_index
  ) {
    b = tx_cmd_buff_o->data[tx_cmd_buff_o->start_index];
    tx_cmd_buff_o->start_index += 1;
  }
  if(tx_cmd_buff_o->start_index==tx_cmd_buff_o->end_index) {
    clear_tx_cmd_buff(tx_cmd_buff_o);
  }
  return b;
}
