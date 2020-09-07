// loopback-rtos.c
// Makes the Tartan Artibeus EXPT board act as a UART loopback using FreeRTOS
//
// Written by Bradley Denby
// Other contributors: None
//
// See the top-level LICENSE file for the license.

// libopencm3
#include <libopencm3/stm32/flash.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/usart.h>

// FreeRTOS
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

// Variables

static QueueHandle_t rxQueue;
static QueueHandle_t txQueue;

// Functions

// Special, optional FreeRTOS function called when a task overruns its stack
extern void vApplicationStackOverflowHook(
 xTaskHandle* pxTask, signed portCHAR* pcTaskName
);

void vApplicationStackOverflowHook(
 xTaskHandle* pxTask __attribute__((unused)),
 signed portCHAR* pcTaskName __attribute__((unused))
) {
  while(true) {}
}

// FreeRTOS rxProducer task: UART RX pin to rxQueue
//   YIELDs when RX pin is empty OR rxQueue is full
static void rxProducer(void *args __attribute__((unused))) {
  while(true) {
    while(
     !usart_get_flag(USART1,USART_ISR_RXNE) ||
     uxQueueSpacesAvailable(rxQueue)==0
    ) {
      taskYIELD();
    }
    char ch = usart_recv(USART1);
    xQueueSend(rxQueue,&ch,portMAX_DELAY);
    gpio_set(GPIOC, GPIO10);
    gpio_clear(GPIOC, GPIO12);
  }
}

// FreeRTOS loopback task: Shift from RX queue to TX queue
//   YIELDs when rxQueue is empty OR txQueue is full
static void loopback(void *args __attribute__((unused))) {
  while(true) {
    while(
     uxQueueMessagesWaiting(rxQueue)==0 || uxQueueSpacesAvailable(txQueue)==0
    ) {
      taskYIELD();
    }
    char ch;
    if(xQueueReceive(rxQueue,&ch,1)==pdPASS) {
      xQueueSend(txQueue,&ch,portMAX_DELAY);
    }
    gpio_clear(GPIOC, GPIO10);
    gpio_clear(GPIOC, GPIO12);
  }
}

// FreeRTOS txConsumer task: txQueue to UART TX pin
//   YIELDs when txQueue is empty OR TX pin is busy
static void txConsumer(void *args __attribute__((unused))) {
  while(true) {
    while(
     !usart_get_flag(USART1,USART_ISR_TXE) ||
     uxQueueMessagesWaiting(txQueue)==0
    ) {
      taskYIELD();
    }
    char ch;
    if(xQueueReceive(txQueue,&ch,1)==pdPASS) {
      usart_send(USART1,ch);
    }
    gpio_clear(GPIOC, GPIO10);
    gpio_set(GPIOC, GPIO12);
  }
}

int main(void) {
  // clock setup
  rcc_osc_on(RCC_HSI16);                    // 16 MHz internal RC oscillator
  rcc_wait_for_osc_ready(RCC_HSI16);        // Wait until oscillator is ready
  rcc_set_sysclk_source(RCC_CFGR_SW_HSI16); // Sets sysclk source for RTOS
  rcc_set_hpre(RCC_CFGR_HPRE_NODIV);        // AHB at 80 MHz (80 MHz max.)
  rcc_set_ppre1(RCC_CFGR_PPRE1_DIV2);       // APB1 at 40 MHz (80 MHz max.)
  rcc_set_ppre2(RCC_CFGR_PPRE2_NODIV);      // APB2 at 80 MHz (80 MHz max.)
  flash_prefetch_enable();                  // Enable instr prefetch buffer
  flash_set_ws(FLASH_ACR_LATENCY_4WS);      // RM0351: 4 WS for 80 MHz
  flash_dcache_enable();                    // Enable data cache
  flash_icache_enable();                    // Enable instruction cache
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
  // LEDs setup
  rcc_periph_clock_enable(RCC_GPIOC);
  gpio_mode_setup(GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO10);
  gpio_mode_setup(GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO12);
  gpio_set(GPIOC, GPIO10);
  gpio_set(GPIOC, GPIO12);
  // UART setup
  rcc_periph_clock_enable(RCC_GPIOA);
  rcc_periph_clock_enable(RCC_USART1);
  gpio_mode_setup(GPIOA,GPIO_MODE_AF,GPIO_PUPD_NONE,GPIO9|GPIO10);
  gpio_set_af(GPIOA,GPIO_AF7,GPIO9);  // USART1_TX and alternate function 7
  gpio_set_af(GPIOA,GPIO_AF7,GPIO10); // USART1_RX and alternate function 7
  usart_set_baudrate(USART1,38400);
  usart_set_databits(USART1,8);
  usart_set_stopbits(USART1,USART_STOPBITS_1);
  usart_set_mode(USART1,USART_MODE_TX_RX);
  usart_set_parity(USART1,USART_PARITY_NONE);
  usart_set_flow_control(USART1,USART_FLOWCONTROL_NONE);
  usart_enable(USART1);
  rxQueue = xQueueCreate(256,sizeof(char));
  txQueue = xQueueCreate(256,sizeof(char));
  // FreeRTOS
  xTaskCreate(rxProducer,"RX",100,NULL,configMAX_PRIORITIES-1,NULL);
  xTaskCreate(loopback,  "LB",100,NULL,configMAX_PRIORITIES-1,NULL);
  xTaskCreate(txConsumer,"TX",100,NULL,configMAX_PRIORITIES-1,NULL);
  vTaskStartScheduler();
  // Loop and return
  while(true) {}
  return 0;
}
