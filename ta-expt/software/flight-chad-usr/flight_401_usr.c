// flight_401_usr.c
// Tartan Artibeus EXPT board flight 401 application main
//
// Written by Bradley Denby
// Other contributors: None
//
// See the top-level LICENSE file for the license.

// Standard library
#include <stddef.h>          // size_t
#include <stdint.h>          // fixed-width integer types

// ta-expt library
#include <application.h>     // microcontroller utility functions
#include <taolst_protocol.h> // protocol utility functions

// Variables

//// in_bootloader is an extern variable read by bootloader_running
int in_bootloader = 0;

//// app_jump_pending is an extern variable used in write_reply
int app_jump_pending = 0;

// Main
int main(void) {
  // Application initialization
  init_clock();
  init_uart();
  init_rtc();
  rx_cmd_buff_t rx_cmd_buff = {.size=CMD_MAX_LEN};
  clear_rx_cmd_buff(&rx_cmd_buff);
  tx_cmd_buff_t tx_cmd_buff = {.size=CMD_MAX_LEN};
  clear_tx_cmd_buff(&tx_cmd_buff);

  // Application loop
  while(1) {
    rx_usart1(&rx_cmd_buff);           // Collect command bytes
    reply(&rx_cmd_buff, &tx_cmd_buff); // Command reply logic
    tx_usart1(&tx_cmd_buff);           // Send a response if any
  }

  // Should never reach this point
  return 0;
}
