// bootloader.h
// Tartan Artibeus EXPT board bootloader header file
//
// Written by Bradley Denby
// Other contributors: None
//
// See the top-level LICENSE file for the license.

#ifndef BOOTLOADER_H
#define BOOTLOADER_H

// Macros

//// Buffer size
#define BUFF_SIZE ((size_t)258)

//// Application start address
#define APP_ADDRESS ((uint32_t)0x08008000U)

//// SRAM1 start address
#define SRAM1_BASE  ((uint32_t)0x20000000U)

//// SRAM1 size
#define SRAM1_SIZE  ((uint32_t)0x00040000U)

// Typedefs
typedef struct fifo_circ_buff {
  size_t        start_index;     // size_t 4 bytes, no padding
  size_t        end_index;       // size_t 4 bytes, no padding
  int           empty;           // int (bool) 4 bytes, no padding
  int           full;            // int (bool) 4 bytes, no padding
  const size_t  size;            // fifo_circ_buff_t buff = {.size=BUFF_SIZE};
  uint8_t       data[BUFF_SIZE]; // may need padding so at the end
} fifo_circ_buff_t;

// Initialization functions
void init_clock(void);
void init_led(void);
void init_uart(void);

// Bootloader functions
int bl_check_app(void);
void bl_jump_to_app(void);

// Helper functions
void init_fifo_circ_buff(fifo_circ_buff_t* buff_o);
int push_fifo_circ_buff(fifo_circ_buff_t* buff_o, const uint8_t b);
int pop_fifo_circ_buff(fifo_circ_buff_t* buff_o, uint8_t* b_o);

// Task-like functions
void rx_usart1(fifo_circ_buff_t* buff_o);
void loopback(fifo_circ_buff_t* rx_buff_o, fifo_circ_buff_t* tx_buff_o);
void tx_usart1(fifo_circ_buff_t* buff_o);

#endif
