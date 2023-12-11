#ifndef __CONSOLE_H
#define __CONSOLE_H

#include "sys.h"

#define LOG_NEWLINE "\r\n"

#define PRINT(fmt, ...)                                                        \
  {                                                                            \
    printf(fmt, ##__VA_ARGS__);                                                \
    fflush(stdout);                                                            \
  }

#define PRINTLN(fmt, ...)                                                      \
  { printf(fmt LOG_NEWLINE, ##__VA_ARGS__); }

// 使用UART1输出
#define CONSOLE_UART USART1
#define CONSOLE_UART_CLK CRM_USART1_PERIPH_CLOCK
#define CONSOLE_UART_IRQ USART1_IRQn
#define CONSOLE_UART_IRQ_HANDLER USART1_IRQHandler

#define CONSOLE_GPIO_CLK CRM_GPIOA_PERIPH_CLOCK
#define CONSOLE_UART_PORT GPIOA
#define CONSOLE_UART_TX_PIN GPIO_PINS_9
#define CONSOLE_UART_RX_PIN GPIO_PINS_10
#define CONSOLE_UART_MUX GPIO_MUX_1
#define CONSOLE_UART_MUX_TX_SRC GPIO_PINS_SOURCE9
#define CONSOLE_UART_MUX_RX_SRC GPIO_PINS_SOURCE10

void console_init(uint32_t baudrate);
void print(const char *str);
void console_write(const void *data, uint32_t len);

#endif //__CONSOLE_H
