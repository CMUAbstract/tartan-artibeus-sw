// bootloader_main.c
// Tests Tartan Artibeus EXPT board bootloader
//
// Written by Bradley Denby
// Other contributors: None
//
// See the top-level LICENSE file for the license.

// ta-expt library
#include <bootloader.h>      // init_*(), *_usart1(), reply(), bl_ops()
#include <taolst_protocol.h> // TAOLST protocol

// Variables
int app_jump_pending;

// Main
int main(void) {
  // Bootloader initialization
  init_clock();
  init_led();
  init_uart();
  rx_cmd_buff_t rx_cmd_buff = {.size=CMD_MAX_LEN};
  clear_rx_cmd_buff(&rx_cmd_buff);
  tx_cmd_buff_t tx_cmd_buff = {.size=CMD_MAX_LEN};
  clear_tx_cmd_buff(&tx_cmd_buff);
  app_jump_pending = 0;
  // Bootloader loop
  while(1) {
    rx_usart1(&rx_cmd_buff);
    reply(&rx_cmd_buff, &tx_cmd_buff);
    tx_usart1(&tx_cmd_buff);
    bl_ops(&tx_cmd_buff, &app_jump_pending);
  }
  return 0;
}
