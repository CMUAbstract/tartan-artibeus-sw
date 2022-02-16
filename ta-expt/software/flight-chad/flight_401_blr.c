// flight_401_blr.c
// Tartan Artibeus EXPT board flight 401 bootloader
//
// Written by Bradley Denby
// Other contributors: None
//
// See the top-level LICENSE file for the license.

// Standard library
#include <stddef.h>          // size_t
#include <stdint.h>          // fixed-width integer types

// ta-expt library
#include <bootloader.h>      // microcontroller utility functions
#include <taolst_protocol.h> // protocol utility functions

// Variables

//// in_bootloader is an extern variable read by bootloader_running
int in_bootloader;

//// app_jump_pending is an extern variable modified in write_reply
int app_jump_pending;

// Main
int main(void) {
  // Bootloader initialization
  init_clock();
  init_uart();
  init_rtc();
  rx_cmd_buff_t rx_cmd_buff = {.size=CMD_MAX_LEN};
  clear_rx_cmd_buff(&rx_cmd_buff);
  tx_cmd_buff_t tx_cmd_buff = {.size=CMD_MAX_LEN};
  clear_tx_cmd_buff(&tx_cmd_buff);
  in_bootloader = 1;
  app_jump_pending = 0;

  // Bootloader loop
  while(1) {
    if(!app_jump_pending) {
      rx_usart1(&rx_cmd_buff);                 // Collect command bytes
      reply(&rx_cmd_buff, &tx_cmd_buff);       // Command reply logic
      tx_usart1(&tx_cmd_buff);                 // Send a response if any
    } else if(bl_check_app()) {                // Jump triggered; do basic check
      while(!tx_cmd_buff.empty) {              // If jumping to user app,
        tx_usart1(&tx_cmd_buff);               // finish sending response if any
      }
      for(size_t i=0; i<4000000; i++) {        // Wait for UART TX FIFO to flush
        __asm__ volatile("nop");
      }
      app_jump_pending = 0;                    // Housekeeping
      in_bootloader = 0;
      bl_jump_to_app();                        // Jump
    } else {                                   // If app_jump_pending &&
      app_jump_pending = 0;                    //  !bl_check_app()
    }                                          // Something wrong, abort jump
  }

  // Should never reach this point
  return 0;
}
