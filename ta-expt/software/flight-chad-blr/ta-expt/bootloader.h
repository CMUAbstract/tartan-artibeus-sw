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
#include <taolst_protocol.h> // rx_cmd_buff_t, tx_cmd_buff_t

// Macros

//// Application start address
#define APP_ADDR   ((uint32_t)0x08008000U)

//// SRAM1 start address
#define SRAM1_BASE ((uint32_t)0x20000000U)

//// SRAM1 size
#define SRAM1_SIZE ((uint32_t)0x00040000U)

// Initialization functions

void init_clock(void);
void init_led(void);
void init_uart(void);
void init_rtc(void);

// Utility functions

/*  int set_rtc(const int32_t sec, const int32_t ns)
 *    sec: seconds since J2000
 *    ns:  any additional nanoseconds
 *  Return:
 *    0 to indicate failure
 *    Non-zero to indicate success
 */
int set_rtc(const uint32_t sec, const uint32_t ns);

/*  int get_rtc(int32_t* sec, int32_t* ns)
 *    sec: Upon return, contains RTC seconds since J2000
 *    ns:  Upon return, contains any additional nanoseconds
 *  Return:
 *    0 to indicate failure (e.g. RTC has not been set)
 *    Non-zero to indicate success (sec and ns contain valid values)
 */
int get_rtc(uint32_t* sec, uint32_t* ns);

// Bootloader functions

int bl_check_app(void);
void bl_jump_to_app(void);

// Task-like functions

void rx_usart1(rx_cmd_buff_t* rx_cmd_buff_o);
void reply(rx_cmd_buff_t* rx_cmd_buff_o, tx_cmd_buff_t* tx_cmd_buff_o);
void tx_usart1(tx_cmd_buff_t* tx_cmd_buff_o);

#endif
