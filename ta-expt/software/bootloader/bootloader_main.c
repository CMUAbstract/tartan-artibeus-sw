// bootloader_main.c
// Tests Tartan Artibeus EXPT board bootloader
//
// Written by Bradley Denby
// Other contributors: None
//
// See the top-level LICENSE file for the license.

// Standard library
#include <stddef.h>                 // NULL
#include <stdint.h>                 // uint8_t

// ta-expt library
#include <bootloader.h>             // init_*(), *_usart1_queue

// Variables
int app_jump_pending;

// Main
int main(void) {
  // Bootloader initialization
  init_clock();
  init_led();
  init_uart();
  fifo_circ_buff_t rx_usart1_buff = {.size=BUFF_SIZE};
  fifo_circ_buff_t tx_usart1_buff = {.size=BUFF_SIZE};
  app_jump_pending = 0;
  // Bootloader loop
  while(1) {
    if(!app_jump_pending) {
      rx_usart1(&rx_usart1_buff);
      loopback(&rx_usart1_buff, &tx_usart1_buff);
      tx_usart1(&tx_usart1_buff);
    } else {
      while(!tx_usart1_buff.empty) {
        tx_usart1(&tx_usart1_buff);
      }
      app_jump_pending = 0;
      bl_jump_to_app();
    }
  }
  return 0;
}
