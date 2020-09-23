// cmd-demo.c
// Tests Tartan Artibeus EXPT board commands demonstration
//
// Written by Bradley Denby
// Other contributors: None
//
// See the top-level LICENSE file for the license.

// Standard library
#include <stddef.h> // size_t

// libopencm3
#include <libopencm3/stm32/flash.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/usart.h>

// FreeRTOS
#include <FreeRTOS.h>
#include <queue.h>
#include <task.h>

// ta-expt library
#include <commands.h>

// Macros
#define PAGE_BUFFER_LEN 128
#define ASCII_BUFFER_LEN 12

// Constants
static const uint16_t HW_ID = 0x0012;

// Variables
static QueueHandle_t txQueue;
static uint16_t msg_id = 0x00;
static char cmd_buffer[CMD_MAX_LEN];
static char page_buffer[PAGE_BUFFER_LEN];
static char ascii_buffer[ASCII_BUFFER_LEN];

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
    gen_app_get_telem(cmd_buffer,CMD_MAX_LEN,HW_ID,msg_id,LST);
    msg_id = (msg_id+1)%65536;
    uart_puts(cmd_buffer,9);
    gpio_toggle(GPIOC,GPIO10);
    gpio_toggle(GPIOC,GPIO12);
    vTaskDelay(pdMS_TO_TICKS(1000));

    gen_app_get_time(cmd_buffer,CMD_MAX_LEN,HW_ID,msg_id,LST);
    msg_id = (msg_id+1)%65536;
    uart_puts(cmd_buffer,9);
    gpio_toggle(GPIOC,GPIO10);
    gpio_toggle(GPIOC,GPIO12);
    vTaskDelay(pdMS_TO_TICKS(1000));

    gen_app_reboot(cmd_buffer,CMD_MAX_LEN,HW_ID,msg_id,LST,1);
    msg_id = (msg_id+1)%65536;
    uart_puts(cmd_buffer,13);
    gpio_toggle(GPIOC,GPIO10);
    gpio_toggle(GPIOC,GPIO12);
    vTaskDelay(pdMS_TO_TICKS(1000));

    gen_app_set_time(
     cmd_buffer,CMD_MAX_LEN,HW_ID,msg_id,LST,653978532,265936000
    );
    msg_id = (msg_id+1)%65536;
    uart_puts(cmd_buffer,17);
    gpio_toggle(GPIOC,GPIO10);
    gpio_toggle(GPIOC,GPIO12);
    vTaskDelay(pdMS_TO_TICKS(1000));

    gen_app_telem(cmd_buffer,CMD_MAX_LEN,HW_ID,msg_id,LST,4);
    msg_id = (msg_id+1)%65536;
    uart_puts(cmd_buffer,13);
    gpio_toggle(GPIOC,GPIO10);
    gpio_toggle(GPIOC,GPIO12);
    vTaskDelay(pdMS_TO_TICKS(1000));

    gen_bootloader_ack(
     cmd_buffer,CMD_MAX_LEN,HW_ID,msg_id,LST,BOOTLOADER_ACK_REASON_PONG
    );
    msg_id = (msg_id+1)%65536;
    uart_puts(cmd_buffer,11);
    gpio_toggle(GPIOC,GPIO10);
    gpio_toggle(GPIOC,GPIO12);
    vTaskDelay(pdMS_TO_TICKS(1000));

    gen_bootloader_erase(cmd_buffer,CMD_MAX_LEN,HW_ID,msg_id,LST,-1);
    msg_id = (msg_id+1)%65536;
    uart_puts(cmd_buffer,9);
    gpio_toggle(GPIOC,GPIO10);
    gpio_toggle(GPIOC,GPIO12);
    vTaskDelay(pdMS_TO_TICKS(1000));

    gen_bootloader_nack(cmd_buffer,CMD_MAX_LEN,HW_ID,msg_id,LST);
    msg_id = (msg_id+1)%65536;
    uart_puts(cmd_buffer,9);
    gpio_toggle(GPIOC,GPIO10);
    gpio_toggle(GPIOC,GPIO12);
    vTaskDelay(pdMS_TO_TICKS(1000));

    gen_bootloader_ping(cmd_buffer,CMD_MAX_LEN,HW_ID,msg_id,LST);
    msg_id = (msg_id+1)%65536;
    uart_puts(cmd_buffer,9);
    gpio_toggle(GPIOC,GPIO10);
    gpio_toggle(GPIOC,GPIO12);
    vTaskDelay(pdMS_TO_TICKS(1000));

    gen_bootloader_write_page(
     cmd_buffer,CMD_MAX_LEN,HW_ID,msg_id,LST,0,page_buffer,PAGE_BUFFER_LEN
    );
    msg_id = (msg_id+1)%65536;
    uart_puts(cmd_buffer,138);
    gpio_toggle(GPIOC,GPIO10);
    gpio_toggle(GPIOC,GPIO12);
    vTaskDelay(pdMS_TO_TICKS(1000));

      // Hack because our bootloader_write_page is slightly different
      gen_common_ack(cmd_buffer,CMD_MAX_LEN,HW_ID,msg_id,LST);
      msg_id = (msg_id+1)%65536;
      uart_puts(cmd_buffer,9);
      gpio_toggle(GPIOC,GPIO10);
      gpio_toggle(GPIOC,GPIO12);
      vTaskDelay(pdMS_TO_TICKS(1000));

    gen_common_ack(cmd_buffer,CMD_MAX_LEN,HW_ID,msg_id,LST);
    msg_id = (msg_id+1)%65536;
    uart_puts(cmd_buffer,9);
    gpio_toggle(GPIOC,GPIO10);
    gpio_toggle(GPIOC,GPIO12);
    vTaskDelay(pdMS_TO_TICKS(1000));

    gen_common_ascii(
     cmd_buffer,CMD_MAX_LEN,HW_ID,msg_id,LST,ascii_buffer,ASCII_BUFFER_LEN
    );
    msg_id = (msg_id+1)%65536;
    uart_puts(cmd_buffer,21);
    gpio_toggle(GPIOC,GPIO10);
    gpio_toggle(GPIOC,GPIO12);
    vTaskDelay(pdMS_TO_TICKS(1000));

    gen_common_nack(cmd_buffer,CMD_MAX_LEN,HW_ID,msg_id,LST);
    msg_id = (msg_id+1)%65536;
    uart_puts(cmd_buffer,9);
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
  usart_set_baudrate(USART1,115200);
  usart_set_databits(USART1,8);
  usart_set_stopbits(USART1,USART_STOPBITS_1);
  usart_set_mode(USART1,USART_MODE_TX);
  usart_set_parity(USART1,USART_PARITY_NONE);
  usart_set_flow_control(USART1,USART_FLOWCONTROL_NONE);
  usart_enable(USART1);
  txQueue = xQueueCreate(256,sizeof(char));
  // Application setup
  for(size_t i=0; i<CMD_MAX_LEN; i++) {
    cmd_buffer[i] = 0x00;
  }
  for(size_t i=0; i<PAGE_BUFFER_LEN; i++) {
    page_buffer[i] = 0xff;
  }
  ascii_buffer[0] = 'H';
  ascii_buffer[1] = 'e';
  ascii_buffer[2] = 'l';
  ascii_buffer[3] = 'l';
  ascii_buffer[4] = 'o';
  ascii_buffer[5] = ' ';
  ascii_buffer[6] = 'w';
  ascii_buffer[7] = 'o';
  ascii_buffer[8] = 'r';
  ascii_buffer[9] = 'l';
  ascii_buffer[10] = 'd';
  ascii_buffer[11] = '!';
  // FreeRTOS
  xTaskCreate(txConsumer,"TX_CONS",100,NULL,configMAX_PRIORITIES-1,NULL);
  xTaskCreate(txProducer,"TX_PROD",100,NULL,configMAX_PRIORITIES-1,NULL);
  vTaskStartScheduler();
  // Loop and return
  while(true) {}
  return 0;
}
