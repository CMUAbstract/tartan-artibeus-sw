// cmd-tx-rtos.c
// Makes the Tartan Artibeus EXPT board send commands using FreeRTOS
//
// Written by Bradley Denby
// Other contributors: None
//
// See the top-level LICENSE file for the license.

// C
#include <stddef.h> // size_t

// libopencm3
#include <libopencm3/stm32/flash.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/usart.h>

// FreeRTOS
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

// Macros
#define OPCODE_COUNT 6
#define CMD_BUF_LEN 251

// Constants
static const uint16_t HWID = 0x12;
static const uint8_t OPCODE[OPCODE_COUNT] = {
 0x17, // APP_GET_TELEM
 0x13, // APP_GET_TIME
 0x0f, // BOOTLOADER_NACK
 0x00, // BOOTLOADER_PING
 0x10, // COMMON_ACK
 0xff  // COMMON_NACK
};

// Variables
static QueueHandle_t txQueue;
static uint8_t opcodeIndex = 0;
static uint16_t seqVal = 0x00;
static char cmdBuffer[CMD_BUF_LEN];

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

// TX consumer task
static void txConsumer(void *args __attribute__((unused))) {
  char ch;
  while(true) {
    if(xQueueReceive(txQueue,&ch,500)==pdPASS) {
      while(!usart_get_flag(USART1,USART_ISR_TXE)) {
        taskYIELD();
      }
      usart_send(USART1,ch);
    }
  }
}

// CMD helper function
static void generateCmd(
 char* cmd, size_t cmdLen, const uint16_t hwid, const uint16_t seqNum,
 const uint8_t dest, const uint8_t opcode
) {
  // Start byte 0
  if(0<cmdLen) {
    cmd[0] = 0x22;
  }
  // Start byte 1
  if(1<cmdLen) {
    cmd[1] = 0x69;
  }
  // Message length
  if(2<cmdLen) {
    cmd[2] = 0x09;
  }
  // HWID least significant byte
  if(3<cmdLen) {
    cmd[3] = (hwid) & 0x00ff;
  }
  // HWID most significant byte
  if(4<cmdLen) {
    cmd[4] = (hwid >> 8) & 0x00ff;
  }
  // Sequence number least significant byte
  if(5<cmdLen) {
    cmd[5] = (seqNum) & 0x00ff;
  }
  // Sequence number most significant byte
  if(6<cmdLen) {
    cmd[6] = (seqNum >> 8) & 0x00ff;
  }
  // Destination
  if(7<cmdLen) {
    cmd[7] = dest;
  }
  // Opcode
  if(8<cmdLen) {
    cmd[8] = opcode;
  }
  // Clear the rest
  for(size_t i=9; i<cmdLen; i++) {
    cmd[i] = 0x00;
  }
}

// TX helper function
static void uart_puts(const char* s, const size_t l) {
  for(size_t i=0; i<l; i++) {
    xQueueSend(txQueue,s,portMAX_DELAY);
    s++;
  }
}

// TX producer task
static void txProducer(void *args __attribute__((unused))) {
  while(true) {
    generateCmd(cmdBuffer,CMD_BUF_LEN,HWID,seqVal,0x01,OPCODE[opcodeIndex]);
    seqVal = (seqVal+1)%65536;
    opcodeIndex = (opcodeIndex+1)%OPCODE_COUNT;
    uart_puts(cmdBuffer,9);
    uart_puts("\r\n",2);
    gpio_toggle(GPIOC,GPIO10);
    gpio_toggle(GPIOC,GPIO12);
    vTaskDelay(pdMS_TO_TICKS(1000));
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
  gpio_clear(GPIOC, GPIO12);
  // UART setup
  rcc_periph_clock_enable(RCC_GPIOA);
  rcc_periph_clock_enable(RCC_USART1);
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
  txQueue = xQueueCreate(256,sizeof(char));
  // Application setup
  for(size_t i=0; i<CMD_BUF_LEN; i++) {
    cmdBuffer[i] = 0x00;
  }
  // FreeRTOS
  xTaskCreate(txConsumer,"TX_CONS",100,NULL,configMAX_PRIORITIES-1,NULL);
  xTaskCreate(txProducer,"TX_PROD",100,NULL,configMAX_PRIORITIES-1,NULL);
  vTaskStartScheduler();
  // Loop and return
  while(true) {}
  return 0;
}
