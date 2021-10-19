// bootloader.c
// Tartan Artibeus EXPT board bootloader implementation file
//
// Written by Bradley Denby
// Other contributors: Shize Che
//
// See the top-level LICENSE file for the license.

// Standard library
#include <stdint.h>                 // uint8_t

// libopencm3 library
#include <libopencm3/cm3/scb.h>     // SCB_VTOR
#include <libopencm3/stm32/flash.h> // used in init_clock
#include <libopencm3/stm32/gpio.h>  // used in init_gpio
#include <libopencm3/stm32/pwr.h>   // used in set_rtc
#include <libopencm3/stm32/rcc.h>   // used in init_clock, init_rtc
#include <libopencm3/stm32/rtc.h>   // used in rtc functions
#include <libopencm3/stm32/usart.h> // used in init_uart

// ta-expt library
#include <bootloader.h>             // Header file
#include <taolst_protocol.h>        // TAOLST protocol macros, typedefs, fnctns

// Variables
int rtc_set = 0; // Boolean; Zero until RTC date and time have been set

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
  gpio_set_af(GPIOA,GPIO_AF7,GPIO9);  // USART1_TX is alternate function 7
  gpio_set_af(GPIOA,GPIO_AF7,GPIO10); // USART1_RX is alternate function 7
  usart_set_baudrate(USART1,115200);
  usart_set_databits(USART1,8);
  usart_set_stopbits(USART1,USART_STOPBITS_1);
  usart_set_mode(USART1,USART_MODE_TX_RX);
  usart_set_parity(USART1,USART_PARITY_NONE);
  usart_set_flow_control(USART1,USART_FLOWCONTROL_NONE);
  usart_enable(USART1);
}

void init_rtc(void) {
  rcc_osc_on(RCC_LSI);               // Low-speed internal oscillator
  rcc_wait_for_osc_ready(RCC_LSI);   // Wait until oscillator is ready
  rcc_periph_clock_enable(RCC_PWR);  // Enable power interface clock for the RTC
  pwr_disable_backup_domain_write_protect();
  rcc_set_rtc_clock_source(RCC_LSI); // Set RTC source
  rcc_enable_rtc_clock();            // Enable RTC
  pwr_enable_backup_domain_write_protect();
  rtc_set = 0;                       // RTC date and time has not yet been set
}

// Utility functions

int set_rtc(const uint32_t sec, const uint32_t ns) {
  // sec and ns represent time since J2000
  //   J2000 UTC: 2000-01-01 11:58:55.816
  // modify sec and ns to indicate time since 2000-01-01 11:58:56
  //   this modification makes the later math simpler
  uint32_t nanosecond  = ns;
  uint32_t sec_rounded = sec;
  if(nanosecond>=184000000) {
    nanosecond -= 184000000;
  } else {
    nanosecond = 1000000000-(184000000-nanosecond);
    if(sec==0) {
      return 0; // refuse to handle this case
    } else {
      sec_rounded -= 1;
    }
  }
  // sec_rounded represents seconds since 2000-01-01 11:58:56
  //   J2000 Julian date: 2451545
  // modify sec_rounded to indicate seconds since 2000-01-01 00:00:00
  //   (Julian date ~2451544.5)
  //   this modification makes the later math simpler
  uint32_t sec_since_y2k = sec_rounded+43136;
  // calculate whole days since Julian date ~2451544.5
  int32_t day_since_y2k = (int32_t)(sec_since_y2k/86400);
  // track the leftover seconds
  uint32_t remaining_sec = sec_since_y2k%86400;
  // calculate the Julian date omitting any remaining_sec
  //   if day_since_y2k==0, jd should be 2451545 b/c remaining_sec<86400
  int32_t jd = 2451545+day_since_y2k;
  // convert jd into year, month, and day (see fliegel1968letters)
  int32_t l = jd+68569;
  int32_t n = 4*l/146097;
  l = l-(146097*n+3)/4;
  int32_t i = 4000*(l+1)/1461001;
  l = l-1461*i/4+31;
  int32_t j = 80*l/2447;
  int32_t k = l-2447*j/80;
  l = j/11;
  j = j+2-12*l;
  i = 100*(n-49)+i+l;
  // convert into uint8_t forms expected by RTC
  uint8_t year = (uint8_t)(i-2000);
  uint8_t month = (uint8_t)(j);
  uint8_t day = (uint8_t)(k);
  // convert remaining_sec into hour, minute, second
  uint8_t hour = (uint8_t)(remaining_sec/3600);
  uint8_t minute = (uint8_t)((remaining_sec%3600)/60);
  uint8_t second = (uint8_t)((remaining_sec%3600)%60);
  // set the RTC
  pwr_disable_backup_domain_write_protect();
  rtc_wait_for_synchro();
  rtc_unlock();
  rtc_set_init_flag();
  rtc_wait_for_init_ready();
  rtc_set_prescaler((uint32_t)249,(uint32_t)127);
  rtc_enable_bypass_shadow_register();
  rtc_calendar_set_year(year);
  rtc_calendar_set_month(month);
  rtc_calendar_set_day(day);
  rtc_set_am_format();
  rtc_time_set_time(hour,minute,second,1);
  rtc_clear_init_flag();
  rtc_lock();
  pwr_enable_backup_domain_write_protect();
  // record and return success
  rtc_set = 1;
  return rtc_set;
}

int get_rtc(uint32_t* sec, uint32_t* ns) {
  if(rtc_set) {
    // Read all values "atomically"
    int32_t year = (int32_t)(((RTC_DR>>20)*10)+((RTC_DR>>16)&0xf)+2000);
    int32_t month = (int32_t)((((RTC_DR>>12)&0x1)*10)+((RTC_DR>>8)&0xf));
    int32_t day = (int32_t)((((RTC_DR>>4)&0x3)*10)+(RTC_DR&0xf));
    int32_t hour = (int32_t)((((RTC_TR>>20)&0x3)*10)+((RTC_TR>>16)&0xf));
    int32_t minute = (int32_t)((((RTC_TR>>12)&0x7)*10)+((RTC_TR>>8)&0xf));
    int32_t second = (int32_t)((((RTC_TR>>4)&0x7)*10)+(RTC_TR&0xf));
    // Calculate JD from year, month, day
    int32_t jd =
     day-32075+1461*(year+4800+(month-14)/12)/4
     +367*(month-2-(month-14)/12*12)/12-3
     *((year+4900+(month-14)/12)/100)/4;
    // Convert into seconds since 2000-01-01 11:58:56
    *sec = (uint32_t)(86400*(jd-2451545)+60*(60*hour+minute)+second-43136);
    // Write nanoseconds
    *ns = (uint32_t)(184000000);
  }
  return rtc_set;
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
  if(                                                  // if
   rx_cmd_buff_o->state==RX_CMD_BUFF_STATE_COMPLETE && // rx_cmd is valid AND
   tx_cmd_buff_o->empty                                // tx_cmd is empty
  ) {                                                  //
    write_reply(rx_cmd_buff_o, tx_cmd_buff_o);         // execute cmd and reply
  }                                                    //
}

void tx_usart1(tx_cmd_buff_t* tx_cmd_buff_o) {
  while(                                             // while
   usart_get_flag(USART1,USART_ISR_TXE) &&           //  USART1 TX empty AND
   !(tx_cmd_buff_o->empty)                           //  TX buffer not empty
  ) {                                                //
    uint8_t b = pop_tx_cmd_buff(tx_cmd_buff_o);      // Pop byte from TX buffer
    usart_send(USART1,b);                            // Send byte to TX pin
  }                                                  //
}
