// bootloader.c
// Tartan Artibeus EXPT board bootloader implementation file
//
// Written by Bradley Denby
// Other contributors: None
//
// See the top-level LICENSE file for the license.

// Standard library
#include <stdint.h>                 // uint8_t

// libopencm3 library
#include <libopencm3/cm3/scb.h>     // SCB_VTOR
#include <libopencm3/stm32/flash.h> // used in init_clock
#include <libopencm3/stm32/gpio.h>  // used in init_gpio
#include <libopencm3/stm32/rcc.h>   // used in init_clock
#include <libopencm3/stm32/usart.h> // used in init_uart

// ta-expt library
#include <bootloader.h>             // macros and function headers
#include <taolst_protocol.h>        // TAOLST protocol struct and functions

// Initialization functions

void init_clock(void) {
  rcc_osc_on(RCC_HSI16);                    // 16 MHz internal RC oscillator
  rcc_wait_for_osc_ready(RCC_HSI16);        // Wait until oscillator is ready
  rcc_set_sysclk_source(RCC_CFGR_SW_HSI16); // Sets sysclk source for RTOS
  rcc_set_hpre(RCC_CFGR_HPRE_NODIV);        // AHB at 80 MHz (80 MHz max.)
  rcc_set_ppre1(RCC_CFGR_PPRE1_DIV2);       // APB1 at 40 MHz (80 MHz max.)
  rcc_set_ppre2(RCC_CFGR_PPRE2_NODIV);      // APB2 at 80 MHz (80 MHz max.)
  //flash_prefetch_enable();                  // Enable instr prefetch buffer
  flash_set_ws(FLASH_ACR_LATENCY_4WS);      // RM0351: 4 WS for 80 MHz
  //flash_dcache_enable();                    // Enable data cache
  //flash_icache_enable();                    // Enable instruction cache
  rcc_set_main_pll(                         // Setup 80 MHz clock
   RCC_PLLCFGR_PLLSRC_HSI16,                // PLL clock source
   4,                                       // PLL VCO division factor
   40,                                      // PLL VCO multiplication factor
   0,                                       // PLL P clk output division factor
   0,                                       // PLL Q clk output division factor
   RCC_PLLCFGR_PLLR_DIV2                    // PLL sysclk output division factor
  ); // 16MHz/4 = 4MHz; 4MHz*40 = 160MHz VCO; 160MHz/2 = 80MHz PLL
  rcc_osc_on(RCC_PLL);                      // 80 MHz PLL
  rcc_wait_for_osc_ready(RCC_PLL);          // Wait until PLL is ready
  rcc_set_sysclk_source(RCC_CFGR_SW_PLL);   // Sets sysclk source for RTOS
  rcc_wait_for_sysclk_status(RCC_PLL);
  rcc_ahb_frequency = 80000000;
  rcc_apb1_frequency = 40000000;
  rcc_apb2_frequency = 80000000;
}

void init_led(void) {
  rcc_periph_clock_enable(RCC_GPIOC);
  gpio_mode_setup(GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO10);
  gpio_mode_setup(GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO12);
  gpio_set(GPIOC, GPIO10);
  gpio_clear(GPIOC, GPIO12);
}

void init_uart(void) {
  rcc_periph_clock_enable(RCC_GPIOA);
  rcc_periph_clock_enable(RCC_USART1);
  gpio_mode_setup(GPIOA,GPIO_MODE_AF,GPIO_PUPD_NONE,GPIO9|GPIO10);
  gpio_set_af(GPIOA,GPIO_AF7,GPIO9);  // USART1_TX and alternate function 7
  gpio_set_af(GPIOA,GPIO_AF7,GPIO10); // USART1_RX and alternate function 7
  usart_set_baudrate(USART1,115200);
  usart_set_databits(USART1,8);
  usart_set_stopbits(USART1,USART_STOPBITS_1);
  usart_set_mode(USART1,USART_MODE_TX_RX);
  usart_set_parity(USART1,USART_PARITY_NONE);
  usart_set_flow_control(USART1,USART_FLOWCONTROL_NONE);
  usart_enable(USART1);
}

// Bootloader functions

int bl_check_app(void) {
  // Does the first four bytes of the application represent the initialization
  // location of a stack pointer within the boundaries of the RAM?
  return (((*(uint32_t*)APP_ADDR)-SRAM1_BASE) <= SRAM1_SIZE);
}

void bl_jump_to_app(void) {
  // The first 4 bytes hold the stack address, so jump address is after that
  uint32_t jump_addr =
   *(volatile uint32_t*)(APP_ADDR+((uint32_t)0x00000004U));
  // Create a jump() function
  void (*jump)(void) = (void (*)(void))jump_addr;
  // Set the vector table
  SCB_VTOR = APP_ADDR;
  // Set the master stack pointer
  __asm__ volatile("msr msp, %0"::"g" (*(volatile uint32_t*)APP_ADDR));
  // Jump to the application
  jump();
}

// Task-like functions

void rx_usart1(rx_cmd_buff_t* rx_cmd_buff_o) {
  while(                                             // while
   usart_get_flag(USART1,USART_ISR_RXNE) &&          //  USART1 RX not empty AND
   rx_cmd_buff_o->state!=RX_CMD_BUFF_STATE_COMPLETE  //  Command not complete
  ) {                                                //
    uint8_t b = usart_recv(USART1);                  // Receive byte from RX pin
    push_rx_cmd_buff(rx_cmd_buff_o, b);              // Push byte to buffer
  }                                                  //
}

void reply(rx_cmd_buff_t* rx_cmd_buff_o, tx_cmd_buff_t* tx_cmd_buff_o) {
  if(
   rx_cmd_buff_o->state==RX_CMD_BUFF_STATE_COMPLETE &&
   tx_cmd_buff_o->empty
  ) {
    write_reply(rx_cmd_buff_o, tx_cmd_buff_o);
  }
}

void tx_usart1(tx_cmd_buff_t* tx_cmd_buff_o) {
  while(                                             // while
   usart_get_flag(USART1,USART_ISR_TXE) &&           //  USART1 TX empty AND
   !(tx_cmd_buff_o->empty)                           //  Buffer not empty
  ) {                                                //
    uint8_t b = pop_tx_cmd_buff(tx_cmd_buff_o);      // Pop byte from buffer
    usart_send(USART1,b);                            // Send byte to TX pin
  }                                                  //
}
