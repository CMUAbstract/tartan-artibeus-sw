// flash_write_demo.c
// Tests Tartan Artibeus EXPT board flash write demonstration
//
// Written by Bradley Denby
// Other contributors: None
//
// See the top-level LICENSE file for the license.

// Standard library
#include <stddef.h>
#include <stdint.h>

// libopencm3 library
#include <libopencm3/stm32/flash.h> // flash_unlock, flash_program_word, etc
#include <libopencm3/stm32/usart.h> // usart_send, USART1

// ta-expt library
#include <application.h>
#include <bootloader.h>
#include <taolst_protocol.h>

// Main
int main(void) {
  // Bootloader initialization

  init_clock();
  init_led();
  init_uart();
  rx_cmd_buff_t rx_cmd_buff = {.size=CMD_MAX_LEN};
  clear_rx_cmd_buff(&rx_cmd_buff);

  // Write program

  flash_unlock();
  flash_erase_page(16);
  flash_clear_status_flags();

  set_bootloader_write_data_cmd(&rx_cmd_buff,0x00,SUBPAGE_00);
  bootloader_write_data(&rx_cmd_buff);
  usart_send(USART1,0x00);
  for(size_t i=0; i<4000000; i++) {
    __asm__ volatile("nop");
  }

  set_bootloader_write_data_cmd(&rx_cmd_buff,0x01,SUBPAGE_01);
  bootloader_write_data(&rx_cmd_buff);
  usart_send(USART1,0x01);
  for(size_t i=0; i<4000000; i++) {
    __asm__ volatile("nop");
  }

  set_bootloader_write_data_cmd(&rx_cmd_buff,0x02,SUBPAGE_02);
  bootloader_write_data(&rx_cmd_buff);
  usart_send(USART1,0x02);
  for(size_t i=0; i<4000000; i++) {
    __asm__ volatile("nop");
  }

  set_bootloader_write_data_cmd(&rx_cmd_buff,0x03,SUBPAGE_03);
  bootloader_write_data(&rx_cmd_buff);
  usart_send(USART1,0x03);
  for(size_t i=0; i<4000000; i++) {
    __asm__ volatile("nop");
  }

  set_bootloader_write_data_cmd(&rx_cmd_buff,0x04,SUBPAGE_04);
  bootloader_write_data(&rx_cmd_buff);
  usart_send(USART1,0x04);
  for(size_t i=0; i<4000000; i++) {
    __asm__ volatile("nop");
  }

  set_bootloader_write_data_cmd(&rx_cmd_buff,0x05,SUBPAGE_05);
  bootloader_write_data(&rx_cmd_buff);
  usart_send(USART1,0x05);
  for(size_t i=0; i<4000000; i++) {
    __asm__ volatile("nop");
  }

  set_bootloader_write_data_cmd(&rx_cmd_buff,0x06,SUBPAGE_06);
  bootloader_write_data(&rx_cmd_buff);
  usart_send(USART1,0x06);
  for(size_t i=0; i<4000000; i++) {
    __asm__ volatile("nop");
  }

  set_bootloader_write_data_cmd(&rx_cmd_buff,0x07,SUBPAGE_07);
  bootloader_write_data(&rx_cmd_buff);
  usart_send(USART1,0x07);
  for(size_t i=0; i<4000000; i++) {
    __asm__ volatile("nop");
  }

  set_bootloader_write_data_cmd(&rx_cmd_buff,0x08,SUBPAGE_08);
  bootloader_write_data(&rx_cmd_buff);
  usart_send(USART1,0x08);
  for(size_t i=0; i<4000000; i++) {
    __asm__ volatile("nop");
  }

  set_bootloader_write_data_cmd(&rx_cmd_buff,0x09,SUBPAGE_09);
  bootloader_write_data(&rx_cmd_buff);
  usart_send(USART1,0x09);
  for(size_t i=0; i<4000000; i++) {
    __asm__ volatile("nop");
  }

  set_bootloader_write_data_cmd(&rx_cmd_buff,0x0a,SUBPAGE_0A);
  bootloader_write_data(&rx_cmd_buff);
  usart_send(USART1,0x0a);
  for(size_t i=0; i<4000000; i++) {
    __asm__ volatile("nop");
  }

  flash_lock();

  // Jump to program

  bl_jump_to_app();

  return 0;
}
