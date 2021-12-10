#include <stdio.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/quadspi.h>
#include <libopencm3/stm32/dma.h>
#include <libopencm3/stm32/flash.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/stm32/dma.h>

void init_clock(void);
void init_led(void);
void init_uart(void);
void init_qspi(void);
void qspi_enable(uint8_t cmd);
void qspi_write_page(uint32_t addr, uint8_t *buf, int len);
uint32_t read_register(void);
void write_register(uint8_t value);

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

/*
 * Quad SPI initialization
 *
 * On this board the following pins are used:
 *	GPIO	Alt Func	Pin Function
 *	 PF6	: AF9  :   QSPI_BK1_IO3
 *	 PF7	: AF9  :   QSPI_BK1_IO2
 *	 PF9	: AF10 :   QSPI_BK1_IO1
 *   PF8	: AF10 :   QSPI_BK1_IO0
 *  PF10	: AF9  :   QSPI_CLK
 *	 PB6	: AF10 :   QSPI_BK1_NCS
 *
 *	This is alt function 10
 *	ST uses a Micro N25Q128A13EF840F 128Mbit QSPI NoR Flash
 *	(so yes 16MB of flash space)
 */
void init_qspi(void) {
  uint32_t cr, dcr, ccr;
  /* enable the RCC clock for the Quad SPI port */
  rcc_periph_clock_enable(RCC_QSPI);
  rcc_periph_reset_hold(RST_QSPI);
  rcc_periph_reset_release(RST_QSPI);
  rcc_periph_clock_enable(RCC_GPIOA);
  rcc_periph_reset_hold(RST_GPIOA);
  rcc_periph_reset_release(RST_GPIOA);
  rcc_periph_clock_enable(RCC_GPIOC);
  rcc_periph_reset_hold(RST_GPIOC);
  rcc_periph_reset_release(RST_GPIOC);

  gpio_set_af(GPIOC, GPIO_AF10, GPIO1); // IO0
  gpio_set_af(GPIOC, GPIO_AF10, GPIO2); // IO1
  gpio_set_af(GPIOC, GPIO_AF10, GPIO3); // IO2
  gpio_set_af(GPIOC, GPIO_AF10, GPIO4); // IO3

  gpio_set_af(GPIOA, GPIO_AF10, GPIO3); // CLK
  gpio_set_af(GPIOC, GPIO_AF10, GPIO11); // BK2_NCS

  gpio_mode_setup(GPIOC, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO1); // IO0
  gpio_mode_setup(GPIOC, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO2); // IO1
  gpio_mode_setup(GPIOC, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO3); // IO2
  gpio_mode_setup(GPIOC, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO4); // IO3

  gpio_mode_setup(GPIOC, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO11); // BK2_NCS
  //gpio_mode_setup(GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLDOWN, GPIO11);
  gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO3); // CLK

  gpio_mode_setup(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLDOWN, GPIO2); // BK1_NCS

  gpio_set_output_options(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_VERYHIGH, GPIO1);
  gpio_set_output_options(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_VERYHIGH, GPIO2);
  gpio_set_output_options(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_VERYHIGH, GPIO3);
  gpio_set_output_options(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_VERYHIGH, GPIO4);
  gpio_set_output_options(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_VERYHIGH, GPIO11);
  
  gpio_set_output_options(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_VERYHIGH, GPIO2);
  gpio_set_output_options(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_VERYHIGH, GPIO3);

  uint32_t tmpreg;
  do {
    tmpreg = (QUADSPI_SR & QUADSPI_SR_BUSY); 
  } while (tmpreg != 0);

  /* quadspi_setup(prescale, shift, fsel?
		  CR - Prescale, Sshift, FSEL, DFM
		  Prescale (ClockPrescaler) = 1
		  SSHIFT (SampleShifting) = 1 (half cycle)
		  FSEL (FlashID) = 0 flash chip 1 selected
		  DFM (DualFlash) = 0 (only 1 flash chip)
	  DCR - Flash Size, Chipselect high time, Clock Mode
		  FSIZE (FlashSize) =  24 (16 MB)
		  CSHT (ChipSelectHighTime) = 1 (2 cycles)
		  CKMODE (ClockMode) = 0 (clock mode 0)
  */
  //QUADSPI_CR = QUADSPI_SET(CR, PRESCALE, 1) | QUADSPI_SET(CR, FTHRES, 3) | QUADSPI_CR_SSHIFT;
  //QUADSPI_DCR = QUADSPI_SET(DCR, FSIZE, 23) | QUADSPI_SET(DCR, CSHT, 1) | QUADSPI_DCR_CKMODE;

  dcr = 0x0;
  dcr |= 0x17 << 16; // FSIZE = 24
  dcr |= 0x1 << 8;   // CSHT = 1
  dcr &= (~0x1);     // CKMODE = 0
  QUADSPI_DCR = dcr;

  cr = 0x0;
  cr |= 0x9 << 24; // Prescale (ClockPrescaler) = 10
  cr |= 0x3 << 8;  // FTHRES = 3
  // cr |= 0x1 << 4;  // SSHIFT = 1
  cr |= 0x1 << 7; // fsel = flash 2
  cr |= 0x1;       // en
  QUADSPI_CR = cr;

  /* enable it qspi_enable() */
  // quadspi_enable();
  // qspi_enable(0x35);
  // qspi_enable(0x66);
  // qspi_enable(0x99);
  ccr = 0x0;
  ccr |= 0xAF;
	ccr |= (QUADSPI_CCR_FMODE_IREAD << QUADSPI_CCR_FMODE_SHIFT);
  ccr |= (QUADSPI_CCR_MODE_1LINE << QUADSPI_CCR_IMODE_SHIFT);
  ccr |= (QUADSPI_CCR_MODE_4LINE << QUADSPI_CCR_DMODE_SHIFT);
  ccr |= (0x8 << 18);
  QUADSPI_DLR = 0x2;
  QUADSPI_CCR = ccr;

  do {
    tmpreg = (QUADSPI_SR & QUADSPI_SR_TCF); 
  } while (tmpreg == 0);

  uint32_t data = QUADSPI_DR;
  QUADSPI_FCR = 0x1f;

  ccr = 0x0;
  ccr |= 0x05;
	ccr |= (QUADSPI_CCR_FMODE_IREAD << QUADSPI_CCR_FMODE_SHIFT);
  ccr |= (QUADSPI_CCR_MODE_1LINE << QUADSPI_CCR_IMODE_SHIFT);
  ccr |= (QUADSPI_CCR_MODE_4LINE << QUADSPI_CCR_DMODE_SHIFT);
  //ccr |= 0x01 << 14;
  ccr |= (0x8 << 18);
  QUADSPI_DLR = 0x1;
  QUADSPI_CCR = ccr;

  do {
    tmpreg = (QUADSPI_SR & QUADSPI_SR_TCF); 
  } while (tmpreg == 0);

  data = QUADSPI_DR;
  QUADSPI_FCR = 0x1f;
  data *= 1;
}

void qspi_enable(uint8_t cmd) {
	uint32_t ccr, sr;
  ccr = 0x0;
  ccr |= cmd;
  ccr |= (QUADSPI_CCR_MODE_4LINE << QUADSPI_CCR_IMODE_SHIFT);
  ccr |= (QUADSPI_CCR_FMODE_IWRITE << QUADSPI_CCR_FMODE_SHIFT);
  QUADSPI_CCR = ccr;
	do {
		sr = QUADSPI_SR;
	} while (sr & QUADSPI_SR_BUSY);
	QUADSPI_FCR = 0x1f; /* reset the flags */
}

uint32_t read_register() {
  uint32_t ccr, sr;
	uint32_t res = 0;

  ccr = 0x0;
  ccr |= 0x05;
	ccr |= (QUADSPI_CCR_FMODE_IREAD << QUADSPI_CCR_FMODE_SHIFT);
  ccr |= (QUADSPI_CCR_MODE_4LINE << QUADSPI_CCR_IMODE_SHIFT);
  ccr |= (QUADSPI_CCR_MODE_4LINE << QUADSPI_CCR_DMODE_SHIFT);
	QUADSPI_DLR = 0;	/* 8 bits */
	QUADSPI_CCR = ccr;
	do {
		res = QUADSPI_DR;
	} while (QUADSPI_SR & QUADSPI_SR_BUSY);
	QUADSPI_FCR = 0x1f;
	return (res & 0xff);
}

void write_register(uint8_t value) {
  uint32_t ccr;
	// uint32_t status;

	qspi_enable(0x06);
  // read_register();
  // usart_send_blocking(USART1, status);

  ccr = 0x0;
  ccr |= 0x01;
  ccr |= (QUADSPI_CCR_FMODE_IWRITE << QUADSPI_CCR_FMODE_SHIFT);
	ccr |= (QUADSPI_CCR_MODE_4LINE << QUADSPI_CCR_IMODE_SHIFT);
  ccr |= (QUADSPI_CCR_MODE_4LINE << QUADSPI_CCR_DMODE_SHIFT);
	QUADSPI_DLR = 0;	/* 8 bits */
	QUADSPI_CCR = ccr;
	QUADSPI_DR = value;
	while (QUADSPI_SR & QUADSPI_SR_BUSY);
	QUADSPI_FCR = 0x1f;
}

void qspi_write_page(uint32_t addr, uint8_t *buf, int len) {
	uint32_t ccr, sr;
	int tmp;

	qspi_enable(0x06);
  
  ccr = QUADSPI_CCR;

  ccr &= ~(QUADSPI_CCR_FMODE_MASK << QUADSPI_CCR_FMODE_SHIFT);
	ccr |= (QUADSPI_CCR_FMODE_IWRITE << QUADSPI_CCR_FMODE_SHIFT);
	/* adjusting this to 0 fixed the write issue. */
	
  // ccr |= QUADSPI_SET(CCR, DCYC, 0); 
  ccr &= ~(QUADSPI_CCR_DCYC_MASK << QUADSPI_CCR_DCYC_SHIFT);
	
  // ccr |= QUADSPI_SET(CCR, INST, 0x32);	/* write 256 bytes */
  ccr &= ~(QUADSPI_CCR_INST_MASK << QUADSPI_CCR_INST_SHIFT);
  ccr |= (0x32 << QUADSPI_CCR_INST_SHIFT);
	
  /* For some reason 1-1-4 command */
	// ccr |= QUADSPI_SET(CCR, IMODE, QUADSPI_CCR_MODE_1LINE);
  ccr &= ~(QUADSPI_CCR_IMODE_MASK << QUADSPI_CCR_IMODE_SHIFT);
	ccr |= (QUADSPI_CCR_MODE_1LINE << QUADSPI_CCR_IMODE_SHIFT);

	// ccr |= QUADSPI_SET(CCR, ADMODE, QUADSPI_CCR_MODE_1LINE);
  ccr &= ~(QUADSPI_CCR_ADMODE_MASK << QUADSPI_CCR_ADMODE_SHIFT);
	ccr |= (QUADSPI_CCR_MODE_1LINE << QUADSPI_CCR_ADMODE_SHIFT);

	// ccr |= QUADSPI_SET(CCR, ADSIZE, 2);	/* 24 bit address */
  ccr &= ~(QUADSPI_CCR_ADSIZE_MASK << QUADSPI_CCR_ADSIZE_SHIFT);
	ccr |= (0x2 << QUADSPI_CCR_ADSIZE_SHIFT);

	// ccr |= QUADSPI_SET(CCR, DMODE, QUADSPI_CCR_MODE_4LINE);
  ccr &= ~(QUADSPI_CCR_DMODE_MASK << QUADSPI_CCR_DMODE_SHIFT);
	ccr |= (QUADSPI_CCR_MODE_4LINE << QUADSPI_CCR_DMODE_SHIFT);

	QUADSPI_DLR = 255;
	QUADSPI_AR = addr;
	QUADSPI_CCR = ccr; /* go write a page */

  tmp = 0;
	do {
		sr = QUADSPI_SR;
		if (sr & QUADSPI_SR_TCF) {
			break;
		}
    tmp++;
    QUADSPI_BYTE_DR = *buf++;
	} while (QUADSPI_SR & QUADSPI_SR_BUSY);

  if (tmp != len) {
    usart_send_blocking(USART1, 0x1);
  }

	QUADSPI_FCR = 0x1f;
}

int main(void) {
  init_clock();
  init_qspi();
  
  /*
  for(int i=0; i<40000; i++) {
      __asm__("nop");
  }*/

  /*
  uint8_t data[256];
  for (int i = 0; i < 256; i++) {
    data[i] = (uint8_t) i;
  }

  qspi_write_page(0x8000000, data, 256);
  */
  //uint8_t st = read_register();

  // usart_send_blocking(USART1, sr);
  // write_register((uint8_t) 1);
  // read_register();
  // usart_send_blocking(USART1, sr);
  //while (1) {}


  return 0;
}