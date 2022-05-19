// sram.c
// Simulation of SRAM random bit-flip due to cosmic radiation
//
// Written by Shize Che
// Other contributors: None
//

// libopencm3
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/flash.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/stm32/syscfg.h>
#include <libopencm3/stm32/rtc.h>
#include <libopencm3/stm32/pwr.h>
#include <libopencm3/stm32/dma.h>
#include <limits.h>
#include <stdlib.h>

#define BYTES_PER_PAGE 2048

uint32_t rng(void);

void init_clock(void);

void init_uart(void);

void init_rtc(void);

void init_sram1(void);

bool rand_error_sram1(uint32_t addr);

int induce_error_sram1(void);

int check_error_sram1(void);

void init_sram2(void);

void sram2_test(void);

void malloc_test(void);

void test(void);

// ideally this, but it uses sram1
// static uint32_t r = 379813; // seed

uint32_t rng() {
  uint32_t a = 16807;
  uint32_t b = 0;
  uint32_t m = UINT_MAX;
  uint32_t r = MMIO32(0x10000000);
  r = (r * a + b) % m;
  MMIO32(0x10000000) = r;
  return r;
}

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

void init_rtc() {
  //procedure to initialize rtc
  rcc_periph_clock_enable(RCC_PWR);
  pwr_disable_backup_domain_write_protect();
  rcc_osc_on(RCC_LSI);
  rcc_wait_for_osc_ready(RCC_LSI);
  rcc_set_rtc_clock_source(RCC_LSI);
  rcc_enable_rtc_clock();
  rtc_wait_for_synchro();
  rtc_unlock();
  rtc_set_init_flag();
  rtc_wait_for_init_ready();
  rtc_set_prescaler((uint32_t) 249, (uint32_t) 127);
  rtc_enable_bypass_shadow_register();

  //set RTC_DR_YT[3:0], Date Register bits [23:20] 
  //and RTC_DR_YU[3:0], Date Register bits [19:16]
  rtc_calendar_set_year((uint8_t) 0);
  
  //set RTC_DR_MT,      Date Register bit  [12] 
  //and RTC_DR_MU[3:0], Date Register bits [11:8]
  rtc_calendar_set_month((uint8_t) 0);

  //set RTC_DR_DT[1:0], Date Register bits [5:4] 
  //and RTC_DR_DU[3:0], Date Register bits [3:0]
  rtc_calendar_set_day((uint8_t) 0);
  
  rtc_set_am_format();
  //set RTC_TR_PM,       Time Register bit  [22]
  //and RTC_TR_HT[3:0],  Time Register bits [21:20]
  //and RTC_TR_HU[3:0],  Time Register bits [19:16]
  //and RTC_TR_MNT[3:0], Time Register bits [14:12]
  //and RTC_TR_MNU[3:0], Time Register bits [11:8]
  //and RTC_TR_ST[3:0],  Time Register bits [6:4]
  //and RTC_TR_SU[3:0],  Time Register bits [3:0]
  rtc_time_set_time((uint8_t) 0, (uint8_t) 0, (uint8_t) 0, true);

  rtc_disable_bypass_shadow_register();
  rtc_clear_init_flag();
  rtc_lock();
  pwr_enable_backup_domain_write_protect();
}

void init_sram1() {
  uint32_t flash_start_addr = 0x08040000; // begining of page 128
  uint32_t sram1_start_addr = 0x20000000; // begining of sram1
  int num_of_words = 128 * 1024 / (sizeof(uint32_t));
  int num_of_dwords = 128 * 1024 / (sizeof(uint32_t) * 2); // 128KB sram1 has this many dwords

  for (int i = 0; i < num_of_words; i++) {
    uint32_t val = rng();
    MMIO32(sram1_start_addr + 4 * i) = val;
    // write value to sram1
    // this causes problem, don't know why
    // MMIO32(sram1_start_addr + 8 * i) = val1;
    // MMIO32(sram1_start_addr + 8 * i + 4) = val2;
    // seems sram can be only written 4 bytes at a time.
    // MMIO64(sram1_start_addr + 8 * i) = val;
  }

  flash_unlock();
  for (int i = 0; i < num_of_dwords; i++) {

    if ((i * sizeof(uint32_t) * 2) % BYTES_PER_PAGE == 0) {
      flash_erase_page(128 + (i * sizeof(uint32_t) * 2) / BYTES_PER_PAGE);
      flash_clear_status_flags();
    }

    uint32_t val1 = MMIO32(sram1_start_addr + 8 * i);
    uint32_t val2 = MMIO32(sram1_start_addr + 8 * i + 4);

    // write value to flash
    flash_wait_for_last_operation();
    FLASH_CR |= FLASH_CR_PG;
    MMIO32(flash_start_addr + 8 * i) = val1;
    MMIO32(flash_start_addr + 8 * i + 4) = val2;
    flash_wait_for_last_operation();
    FLASH_CR &= ~FLASH_CR_PG;
    flash_clear_status_flags();
  }
  // usart_send_blocking(USART1, 0x42);
  flash_lock();
  // usart_send_blocking(USART1, 0x43);
}

bool rand_error_sram1(uint32_t addr) {
  uint32_t rand = rng();

  if (rand < UINT_MAX / 100) {
    char new_val;
    uint32_t new_word;
    uint32_t word = MMIO32(addr / 4 * 4);

    uint32_t r = rng();
    int shift = (r / (UINT_MAX / 8)) % 8;
    new_word = word ^ (0x1 << (shift + ((3 - (addr % 4)) * 8)));

    // make sure new_word is not same as word, shouldn't see these
    if (new_word == word) {
      usart_send_blocking(USART1, 0xee);
      usart_send_blocking(USART1, shift);
      usart_send_blocking(USART1, word >> 24);
      usart_send_blocking(USART1, word >> 16);
      usart_send_blocking(USART1, word >> 8);
      usart_send_blocking(USART1, word);
      usart_send_blocking(USART1, new_word >> 24);
      usart_send_blocking(USART1, new_word >> 16);
      usart_send_blocking(USART1, new_word >> 8);
      usart_send_blocking(USART1, new_word);
    }

    MMIO32(addr / 4 * 4) = new_word;
    return true;
  }

  return false;
}

int induce_error_sram1() {
  uint32_t flash_start_addr = 0x08060000; // begining of page 192
  uint32_t sram1_start_addr = 0x20000000; // begining of sram1
  int num_of_bytes = 128 * 1024 / sizeof(char); // 128KB sram1 has this many bytes
  int num_induced_addr = 0;
  uint32_t buff[2];
  int buff_idx = 0;

  flash_unlock();
  for (int i = 0; i < num_of_bytes; i++) {
    if (rand_error_sram1(sram1_start_addr + i)) {
      buff[buff_idx] = sram1_start_addr + i;
      num_induced_addr++;
      buff_idx++;
      // usart_send_blocking(USART1, i);
    }
    if (buff_idx == 2) {
      if ((num_induced_addr * sizeof(char *)) % BYTES_PER_PAGE == 0) {
        flash_erase_page(192 + (num_induced_addr * sizeof(char *)) / BYTES_PER_PAGE);
        flash_clear_status_flags();
      }

      // write error-induced address to flash starting at page 192
      flash_wait_for_last_operation();
      FLASH_CR |= FLASH_CR_PG;
      MMIO32(flash_start_addr + 4 * num_induced_addr) = buff[0];
      MMIO32(flash_start_addr + 4 * num_induced_addr + 4) = buff[1];
      flash_wait_for_last_operation();
      FLASH_CR &= ~FLASH_CR_PG;
      flash_clear_status_flags();

      buff_idx = 0;
     
      /*
      usart_send_blocking(USART1, (int) buff[0] >> 24);
      usart_send_blocking(USART1, (int) buff[0] >> 16);
      usart_send_blocking(USART1, (int) buff[0] >> 8);
      usart_send_blocking(USART1, (int) buff[0]);
      usart_send_blocking(USART1, (int) buff[1] >> 24);
      usart_send_blocking(USART1, (int) buff[1] >> 16);
      usart_send_blocking(USART1, (int) buff[1] >> 8);
      usart_send_blocking(USART1, (int) buff[1]);
      */
    }
  }
  flash_lock();

  return num_induced_addr;
}

int check_error_sram1() {
  uint32_t flash_start_addr = 0x08040000; // begining of page 128
  uint32_t sram1_start_addr = 0x20000000; // begining of sram1
  int num_of_bytes = 128 * 1024 / sizeof(char); // 256KB sram1 has this many bytes
  int num_errors = 0;

  for (int i = 0; i < num_of_bytes; i++) {
    uint32_t word = MMIO32(sram1_start_addr + i / 4 * 4);
    char current_val = word >> ((3 - (i % 4)) * 8);
    uint64_t word1 = MMIO32(flash_start_addr + i / 8 * 8);
    uint64_t word2 = MMIO32(flash_start_addr + i / 8 * 8 + 4);
    uint64_t dword = (word1 << 32) + word2;
    char original_val = dword >> ((7 - (i % 8)) * 8);
    if (current_val != original_val) {
      num_errors++;
      /*
      usart_send_blocking(USART1, i);
      usart_send_blocking(USART1, 0xee);
      usart_send_blocking(USART1, current_val);
      usart_send_blocking(USART1, original_val);
      */
    }
  }
  // usart_send_blocking(USART1, 0x05);

  return num_errors;
}

void init_sram2() {
  uint32_t flash_start_addr = 0x08040000; // begining of page 128
  uint32_t sram2_start_addr = 0x10000000; // begining of sram2
  int num_of_words = 64 * 1024 / (sizeof(uint32_t));
  int num_of_dwords = 64 * 1024 / (sizeof(uint32_t) * 2); // 64KB sram2 has this many dwords

  for (int i = 0; i < num_of_words; i++) {
    uint32_t val = rng();
    MMIO32(sram2_start_addr + 4 * i) = val;
    // write value to sram1
    // this causes problem, don't know why
    // MMIO32(sram1_start_addr + 8 * i) = val1;
    // MMIO32(sram1_start_addr + 8 * i + 4) = val2;
    // seems sram can be only written 4 bytes at a time.
    // MMIO64(sram1_start_addr + 8 * i) = val;
  }

  flash_unlock();
  for (int i = 0; i < num_of_dwords; i++) {

    if ((i * sizeof(uint32_t) * 2) % BYTES_PER_PAGE == 0) {
      flash_erase_page(128 + (i * sizeof(uint32_t) * 2) / BYTES_PER_PAGE);
      flash_clear_status_flags();
    }

    uint32_t val1 = MMIO32(sram2_start_addr + 8 * i);
    uint32_t val2 = MMIO32(sram2_start_addr + 8 * i + 4);

    // write value to flash
    flash_wait_for_last_operation();
    FLASH_CR |= FLASH_CR_PG;
    MMIO32(flash_start_addr + 8 * i) = val1;
    MMIO32(flash_start_addr + 8 * i + 4) = val2;
    flash_wait_for_last_operation();
    FLASH_CR &= ~FLASH_CR_PG;
    flash_clear_status_flags();
  }
  // usart_send_blocking(USART1, 0x42);
  flash_lock();
}

void sram2_test() {
  rcc_periph_clock_enable(RCC_SYSCFG);
  rcc_periph_clock_enable(RCC_FLASH);
  flash_unlock();

  flash_program_option_bytes(0xEFEFF8AA);

  for (uint32_t ptr = 0x10000000; ptr < 0x10010000; ptr++) {
    uint32_t val = MMIO32(ptr / 4 * 4);
    usart_send_blocking(USART1, SYSCFG_CFGR2 >> 8);
    // usart_send_blocking(USART1, val);
  }
  /*
  for (char *ptr = (char *)0x20040000; ptr < (char *)0x20050000; ptr++) {
    char val = *ptr;
    usart_send_blocking(USART1, val);
  }*/
}

void malloc_test() {
  char *addr = malloc(sizeof(char));
  usart_send_blocking(USART1, (int) addr >> 24);
  usart_send_blocking(USART1, (int) addr >> 16);
  usart_send_blocking(USART1, (int) addr >> 8);
  usart_send_blocking(USART1, (int) addr);
}

void test(void) {
  flash_erase_page(255);
  flash_clear_status_flags();
  flash_unlock();
  flash_wait_for_last_operation();
  FLASH_CR |= FLASH_CR_PG;
  MMIO32(0x0807f800) = 0x12345678;
  MMIO32(0x0807f804) = 0x87654321;
  flash_wait_for_last_operation();
  FLASH_CR &= ~FLASH_CR_PG;
  flash_clear_status_flags();
  flash_lock();
  uint32_t x = 0xffffffff;
  x = * (uint32_t *) 0x0807f800;
  usart_send_blocking(USART1, x >> 24);
  usart_send_blocking(USART1, x >> 16);
  usart_send_blocking(USART1, x >> 8);
  usart_send_blocking(USART1, x);
  uint32_t y = 0x11111111;
  y = * (uint32_t *) 0x0807f804;
  usart_send_blocking(USART1, y >> 24);
  usart_send_blocking(USART1, y >> 16);
  usart_send_blocking(USART1, y >> 8);
  usart_send_blocking(USART1, y);
}

int see_sram1(void);

int see_sram1() {
  uint32_t sram1_start_addr = 0x20000000; // begining of sram1
  int num_of_bytes = 128 * 1024 / sizeof(char); // 256KB sram1 has this many bytes
  int count = 0;

  for (int i = 0; i < num_of_bytes; i++) {
    uint32_t word = MMIO32(sram1_start_addr + i / 4 * 4);
    char current_val = word >> ((3 - (i % 4)) * 8);
    // usart_send_blocking(USART1, current_val);
    if ((i % 4 == 0 && current_val != 0x12) ||
        (i % 4 == 1 && current_val != 0x34) ||
        (i % 4 == 2 && current_val != 0x56) ||
        (i % 4 == 3 && current_val != 0x78)) {
      count++;
      uint32_t addr = sram1_start_addr + i / 4 * 4;
      /*
      usart_send_blocking(USART1, addr >> 24);
      usart_send_blocking(USART1, addr >> 16);
      usart_send_blocking(USART1, addr >> 8);
      usart_send_blocking(USART1, addr);
      */
    }
  }
  return count;
}

int see_flash(void);

int see_flash() {
  uint32_t flash_start_addr = 0x08040000; // begining of page 128
  int num_of_bytes = 128 * 1024 / sizeof(char); // 256KB sram1 has this many bytes
  int count = 0;

  for (int i = 0; i < num_of_bytes; i++) {
    uint64_t word1 = MMIO32(flash_start_addr + i / 8 * 8);
    uint64_t word2 = MMIO32(flash_start_addr + i / 8 * 8 + 4);
    uint64_t dword = (word1 << 32) + word2;
    char original_val = dword >> ((7 - (i % 8)) * 8);
    if ((i % 4 == 0 && original_val != 0x12) ||
        (i % 4 == 1 && original_val != 0x34) ||
        (i % 4 == 2 && original_val != 0x56) ||
        (i % 4 == 3 && original_val != 0x78)) {
      count++;
      /*
      uint32_t addr = flash_start_addr + i / 4 * 4;
      usart_send_blocking(USART1, addr >> 24);
      usart_send_blocking(USART1, addr >> 16);
      usart_send_blocking(USART1, addr >> 8);
      usart_send_blocking(USART1, addr);
      */
    }
  }

  return count;
}

void print_int(int n);

void print_int(int n) {
  uint8_t buff[10];
  int i = 0;
  int num_digits = 0;
  while (n > 0) {
    uint8_t digit = n % 10;
    buff[i] = digit;
    n /= 10;
    num_digits++;
    i++;
  }
  for (int j = num_digits - 1; j >= 0; j--) {
    usart_send_blocking(USART1, buff[j] + 48);
  }
}

void pretty_print(int num_induced, int num_found, int time_spent);

void pretty_print(int num_induced, int num_found, int time_spent) {
  usart_send_blocking(USART1, '\r');
  usart_send_blocking(USART1, '\n');
  usart_send_blocking(USART1, '#');
  usart_send_blocking(USART1, 'e');
  usart_send_blocking(USART1, 'r');
  usart_send_blocking(USART1, 'r');
  usart_send_blocking(USART1, 'o');
  usart_send_blocking(USART1, 'r');
  usart_send_blocking(USART1, 's');
  usart_send_blocking(USART1, ' ');
  usart_send_blocking(USART1, 'i');
  usart_send_blocking(USART1, 'n');
  usart_send_blocking(USART1, 'd');
  usart_send_blocking(USART1, 'u');
  usart_send_blocking(USART1, 'c');
  usart_send_blocking(USART1, 'e');
  usart_send_blocking(USART1, 'd');
  usart_send_blocking(USART1, ':');
  usart_send_blocking(USART1, ' ');
  print_int(num_induced);
  usart_send_blocking(USART1, '\r');
  usart_send_blocking(USART1, '\n');
  usart_send_blocking(USART1, '#');
  usart_send_blocking(USART1, 'e');
  usart_send_blocking(USART1, 'r');
  usart_send_blocking(USART1, 'r');
  usart_send_blocking(USART1, 'o');
  usart_send_blocking(USART1, 'r');
  usart_send_blocking(USART1, 's');
  usart_send_blocking(USART1, ' ');
  usart_send_blocking(USART1, 'f');
  usart_send_blocking(USART1, 'o');
  usart_send_blocking(USART1, 'u');
  usart_send_blocking(USART1, 'n');
  usart_send_blocking(USART1, 'd');
  usart_send_blocking(USART1, ':');
  usart_send_blocking(USART1, ' ');
  print_int(num_found);
  usart_send_blocking(USART1, '\r');
  usart_send_blocking(USART1, '\n');
  usart_send_blocking(USART1, 't');
  usart_send_blocking(USART1, 'i');
  usart_send_blocking(USART1, 'm');
  usart_send_blocking(USART1, 'e');
  usart_send_blocking(USART1, ' ');
  usart_send_blocking(USART1, 's');
  usart_send_blocking(USART1, 'p');
  usart_send_blocking(USART1, 'e');
  usart_send_blocking(USART1, 'n');
  usart_send_blocking(USART1, 't');
  usart_send_blocking(USART1, ':');
  usart_send_blocking(USART1, ' ');
  print_int(time_spent);
  usart_send_blocking(USART1, 's');
  usart_send_blocking(USART1, '\r');
  usart_send_blocking(USART1, '\n');
}

// DMA code
void dma_demo(void);

// Demo of transfering four words from SRAM to flash using DMA
void dma_demo() {
  MMIO32(0x10000000) = 0x87654321;
  MMIO32(0x10000004) = 0x87654321;
  MMIO32(0x10000008) = 0x87654321;
  MMIO32(0x1000000C) = 0x87654321;
  rcc_periph_clock_enable(RCC_DMA1);
  DMA1_CPAR1 = 0x10000000; // source
  DMA1_CMAR1 = 0x08040000; // destination
  DMA1_CNDTR1 = 0x4; // number of data to transfer
  DMA1_CCR1 |= 0x1 << 14; // set memory-to-memory mode
  DMA1_CCR1 |= 0x2 << 10; // source transfer unit
  DMA1_CCR1 |= 0x2 << 8; // destination transfer unit
  DMA1_CCR1 |= 0x1 << 7; // set source incremental mode
  DMA1_CCR1 |= 0x1 << 6; // set destination incremental mode
  flash_unlock();
  flash_erase_page(128);
  flash_clear_status_flags();
  flash_wait_for_last_operation();
  FLASH_CR |= FLASH_CR_PG;
  DMA1_CCR1 |= 0x1;
  do {
    __asm__("nop");
  } while (!(DMA1_ISR & 0x2));

  flash_wait_for_last_operation();
  FLASH_CR &= ~FLASH_CR_PG;
  flash_clear_status_flags();
  flash_lock();
  usart_send_blocking(USART1, DMA1_ISR & 0xf);
  int x = MMIO32(0x08040000);
  int y = MMIO32(0x08040004);
  int z = MMIO32(0x08040008);
  int u = MMIO32(0x0804000C);
  usart_send_blocking(USART1, x >> 24);
  usart_send_blocking(USART1, x >> 16);
  usart_send_blocking(USART1, x >> 8);
  usart_send_blocking(USART1, x);
  usart_send_blocking(USART1, y >> 24);
  usart_send_blocking(USART1, y >> 16);
  usart_send_blocking(USART1, y >> 8);
  usart_send_blocking(USART1, y);
  usart_send_blocking(USART1, z >> 24);
  usart_send_blocking(USART1, z >> 16);
  usart_send_blocking(USART1, z >> 8);
  usart_send_blocking(USART1, z);
  usart_send_blocking(USART1, u >> 24);
  usart_send_blocking(USART1, u >> 16);
  usart_send_blocking(USART1, u >> 8);
  usart_send_blocking(USART1, u);
}

void dma_read_page(void);

void dma_read_page() {
  uint32_t flash_start_addr = 0x08040000;
  uint32_t sram1_start_addr = 0x20000000;
  flash_unlock();
  flash_erase_page(128);
  flash_clear_status_flags();
  flash_wait_for_last_operation();
  FLASH_CR |= FLASH_CR_PG;
  for (uint32_t i = 0; i < BYTES_PER_PAGE / sizeof(uint64_t); i++) {
    MMIO32(flash_start_addr + i * sizeof(uint64_t)) = 0x12345678;
    MMIO32(flash_start_addr + i * sizeof(uint64_t) + 4) = 0x12345678;
  }
  flash_wait_for_last_operation();
  FLASH_CR &= ~FLASH_CR_PG;
  flash_clear_status_flags();
  flash_lock();

  usart_send_blocking(USART1, 0x42);

  rcc_periph_clock_enable(RCC_DMA1);
  DMA1_CPAR1 = 0x08040000; // source
  DMA1_CMAR1 = 0x20000000; // destination
  DMA1_CNDTR1 = BYTES_PER_PAGE / sizeof(uint32_t); // number of data to transfer
  DMA1_CCR1 |= 0x1 << 14; // set memory-to-memory mode
  DMA1_CCR1 |= 0x2 << 10; // source transfer unit 32bits
  DMA1_CCR1 |= 0x2 << 8; // destination transfer unit 32bits
  DMA1_CCR1 |= 0x1 << 7; // set source incremental mode
  DMA1_CCR1 |= 0x1 << 6; // set destination incremental mode
  DMA1_CCR1 |= 0x1; // start

  do {
    __asm__("nop");
  } while (!(DMA1_ISR & 0x2));

  usart_send_blocking(USART1, 0x43);

  for (uint32_t i = 0; i < BYTES_PER_PAGE / sizeof(uint64_t); i++) {
    uint32_t x = MMIO32(sram1_start_addr + i * sizeof(uint64_t));
    uint32_t y = MMIO32(sram1_start_addr + i * sizeof(uint64_t) + 4);
    if (x != 0x12345678)
      usart_send_blocking(USART1, 0xee);
    if (y != 0x12345678)
      usart_send_blocking(USART1, 0xee);
  }

  usart_send_blocking(USART1, 0x44);
}

void dma_write_page(void);

void dma_write_page() {
  uint32_t flash_start_addr = 0x08040000;
  uint32_t sram1_start_addr = 0x20000000;
  for (uint32_t i = 0; i < BYTES_PER_PAGE / sizeof(uint64_t); i++) {
    MMIO32(sram1_start_addr + i * sizeof(uint64_t)) = 0x12345678;
    MMIO32(sram1_start_addr + i * sizeof(uint64_t) + 4) = 0x12345678;
  }
  usart_send_blocking(USART1, 0x42);
  rcc_periph_clock_enable(RCC_DMA1);
  DMA1_CPAR1 = 0x20000000; // source
  DMA1_CMAR1 = 0x08040000; // destination
  DMA1_CNDTR1 = BYTES_PER_PAGE / sizeof(uint32_t); // number of data to transfer
  DMA1_CCR1 |= 0x1 << 14; // set memory-to-memory mode
  DMA1_CCR1 |= 0x2 << 10; // source transfer unit
  DMA1_CCR1 |= 0x2 << 8; // destination transfer unit
  DMA1_CCR1 |= 0x1 << 7; // set source incremental mode
  DMA1_CCR1 |= 0x1 << 6; // set destination incremental mode
  flash_unlock();
  flash_erase_page(128);
  flash_clear_status_flags();
  flash_wait_for_last_operation();
  FLASH_CR |= FLASH_CR_PG;
  DMA1_CCR1 |= 0x1;
  do {
    __asm__("nop");
  } while (!(DMA1_ISR & 0x2));

  flash_wait_for_last_operation();
  FLASH_CR &= ~FLASH_CR_PG;
  flash_clear_status_flags();

  usart_send_blocking(USART1, 0x43);

  for (uint32_t i = 0; i < BYTES_PER_PAGE / sizeof(uint64_t); i++) {
    uint32_t x = MMIO32(flash_start_addr + i * sizeof(uint64_t));
    uint32_t y = MMIO32(flash_start_addr + i * sizeof(uint64_t) + 4);
    if (x != 0x12345678)
      usart_send_blocking(USART1, 0xee);
    if (y != 0x12345678)
      usart_send_blocking(USART1, 0xee);
  }

  usart_send_blocking(USART1, 0x44);
}

int main(void) {
  init_clock();
  init_uart();
  // init_rtc();
  // sram2_test();

  // use sram2 to store rng seed because global variable defaults to sram1
  // and interferes the experiment
  // MMIO32(0x10000000) = (uint32_t) 2746;
  // init_sram1();
  // usart_send_blocking(USART1, 0x44);

  /*
  int count = see_sram1();
  usart_send_blocking(USART1, count >> 24);
  usart_send_blocking(USART1, count >> 16);
  usart_send_blocking(USART1, count >> 8);
  usart_send_blocking(USART1, count);

  int count = see_flash();
  usart_send_blocking(USART1, count >> 24);
  usart_send_blocking(USART1, count >> 16);
  usart_send_blocking(USART1, count >> 8);
  usart_send_blocking(USART1, count);
  */

  // malloc_test();
  /*
  int num_induced = induce_error_sram1();
  uint8_t tr1 = RTC_TR;
  int num_found = check_error_sram1();
  uint8_t tr2 = RTC_TR;
  int start_time = (tr1 >> 4) * 10 + (tr1 & 0xf);
  int end_time = (tr2 >> 4) * 10 + (tr2 & 0xf);
  int time_spent = end_time - start_time;
  pretty_print(num_induced, num_found, time_spent);
  */
  dma_write_page();
  
  return 0;
}
