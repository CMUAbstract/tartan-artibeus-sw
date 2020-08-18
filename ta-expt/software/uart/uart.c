// uart.c
// Makes the Tartan Artibeus EXPT board UART go
//
// Written by Bradley Denby
// Other contributors: None
//
// See the top-level LICENSE file for the license.

// libopencm3
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/usart.h>

int main(void) {
  // clock setup
  rcc_osc_on(RCC_HSI16);   // high-speed (16 MHz) internal RC oscillator
  //flash_prefetch_enable(); // buffer used for instruction fetches
  //flash_set_ws(4);         // number of wait states should be matched to clock
  //flash_dcache_enable();   // enable data cache
  //flash_icache_enable();   // enable instruction cache
    //// 16MHz/4 = 4MHz; 4*40=160MHz VCO; 80MHz main PLL
  rcc_set_main_pll(RCC_PLLCFGR_PLLSRC_HSI16,4,40,0,0,RCC_PLLCFGR_PLLR_DIV2);
  rcc_osc_on(RCC_PLL);     // internal phase-locked loop
    //// At this point, either rcc_wait_for_osc_ready() or do other things
	rcc_periph_clock_enable(RCC_GPIOA);  // enable clock for port with USART pins
	rcc_periph_clock_enable(RCC_GPIOC);  // enable clock for port with LED pins
  rcc_periph_clock_enable(RCC_USART1); // enable clock for USART1 peripheral
  // USART setup
    //// Setup GPIO pins for TX and RX
  gpio_mode_setup(GPIOA,GPIO_MODE_AF,GPIO_PUPD_NONE,GPIO9|GPIO10);
  gpio_set_af(GPIOA,GPIO_AF7,GPIO9);  // USART1_TX and alternate function 7
  gpio_set_af(GPIOA,GPIO_AF7,GPIO10); // USART1_RX and alternate function 7
	usart_set_baudrate(USART1,38400);
	usart_set_databits(USART1,8);
	usart_set_stopbits(USART1,USART_STOPBITS_1);
	usart_set_mode(USART1,USART_MODE_TX);
	usart_set_parity(USART1,USART_PARITY_NONE);
	usart_set_flow_control(USART1,USART_FLOWCONTROL_NONE);
	usart_enable(USART1);
  // LED for ticking
  gpio_mode_setup(GPIOC,GPIO_MODE_OUTPUT,GPIO_PUPD_NONE,GPIO10);
  gpio_clear(GPIOC,GPIO10);
  // Loop
  while(1) {
    usart_send_blocking(USART1,'H');
    usart_send_blocking(USART1,'e');
    usart_send_blocking(USART1,'l');
    usart_send_blocking(USART1,'l');
    usart_send_blocking(USART1,'o');
    usart_send_blocking(USART1,' ');
    usart_send_blocking(USART1,'w');
    usart_send_blocking(USART1,'o');
    usart_send_blocking(USART1,'r');
    usart_send_blocking(USART1,'l');
    usart_send_blocking(USART1,'d');
    usart_send_blocking(USART1,'!');
    usart_send_blocking(USART1,'\r');
    usart_send_blocking(USART1,'\n');
    gpio_toggle(GPIOC,GPIO10);
    for(int i=0; i<800000; i++) {
      __asm__("nop");
    }
  }
}
