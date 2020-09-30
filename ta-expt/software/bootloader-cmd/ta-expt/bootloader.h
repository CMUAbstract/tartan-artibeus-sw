// bootloader.h
// Tartan Artibeus EXPT board bootloader header file
//
// Written by Bradley Denby
// Other contributors: None
//
// See the top-level LICENSE file for the license.

#ifndef BOOTLOADER_H
#define BOOTLOADER_H

// ta-expt library
#include <taolst_protocol.h> // TAOLST protocol macros, typedefs, functions

// Macros

//// Application start address
#define APP_ADDRESS ((uint32_t)0x08008000U)

//// SRAM1 start address
#define SRAM1_BASE  ((uint32_t)0x20000000U)

//// SRAM1 size
#define SRAM1_SIZE  ((uint32_t)0x00040000U)

// Initialization functions

void init_clock(void);
void init_led(void);
void init_uart(void);

// Bootloader functions

int bl_check_app(void);
void bl_jump_to_app(void);

// Task-like functions

void rx_usart1(rx_cmd_buff_t* rx_cmd_buff_o);
void reply(rx_cmd_buff_t* rx_cmd_buff_o, tx_cmd_buff_t* tx_cmd_buff_o);
void tx_usart1(tx_cmd_buff_t* tx_cmd_buff_o);
void bl_ops(const tx_cmd_buff_t* tx_cmd_buff, int* app_jump_pending_o);

#endif
