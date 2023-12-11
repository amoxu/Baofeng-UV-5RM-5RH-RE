#include "console.h"
#include "sys.h"

#if defined(__GNUC__) && !defined(__clang__)
/* With GCC/RAISONANCE, small printf (option LD Linker->Libraries->Small printf
     set to 'Yes') calls __io_putchar() */
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */

/**
 * @brief  Retargets the C library printf function to the USART.
 * @param  None
 * @retval None
 */
PUTCHAR_PROTOTYPE {
  /* Loop until the end of transmission */
  while (usart_flag_get(CONSOLE_UART, USART_TDBE_FLAG) == RESET)
    ;
  /* e.g. write a character to the UART */
  usart_data_transmit(CONSOLE_UART, ch);
  return ch;
}

void print(const char *str) { console_write(str, strlen(str)); }

void console_write(const void *data, uint32_t len) {
  const uint8_t *d = data;
  for (; len > 0; --len, ++d) {
    /* Loop until the end of transmission */
    while (usart_flag_get(CONSOLE_UART, USART_TDBE_FLAG) == RESET)
      ;
    usart_data_transmit(CONSOLE_UART, *d);
  }
}

// Initialize the console UART
void console_init(uint32_t baudrate) {
  crm_periph_clock_enable(CONSOLE_UART_CLK, TRUE); // enable GPIO clock
  crm_periph_clock_enable(CONSOLE_GPIO_CLK, TRUE); // enable USART clock

  // gpio init
  gpio_init_type gpioinit;
  gpioinit.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
  gpioinit.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
  gpioinit.gpio_mode = GPIO_MODE_MUX;
  gpioinit.gpio_pins = CONSOLE_UART_TX_PIN | CONSOLE_UART_RX_PIN;
  gpioinit.gpio_pull = GPIO_PULL_NONE;
  gpio_init(CONSOLE_UART_PORT, &gpioinit);

  // gpio mux config
  gpio_pin_mux_config(CONSOLE_UART_PORT, CONSOLE_UART_MUX_TX_SRC,
                      CONSOLE_UART_MUX);
  gpio_pin_mux_config(CONSOLE_UART_PORT, CONSOLE_UART_MUX_RX_SRC,
                      CONSOLE_UART_MUX);

  // USART Init
  uint32_t apb_clock, temp_val;
  apb_clock = 8000000;

  temp_val = (apb_clock * 10 / baudrate);
  if ((temp_val % 10) < 5) {
    temp_val = (temp_val / 10);
  } else {
    temp_val = (temp_val / 10) + 1;
  }
  CONSOLE_UART->baudr_bit.div = temp_val;
  CONSOLE_UART->ctrl1_bit.dbn = USART_DATA_8BITS;
  CONSOLE_UART->ctrl2_bit.stopbn = USART_STOP_1_BIT;

  // parity none
  CONSOLE_UART->ctrl1_bit.psel = FALSE;
  CONSOLE_UART->ctrl1_bit.pen = FALSE;

  // enable tx
  CONSOLE_UART->ctrl1_bit.ten = TRUE;

  // enable uart
  CONSOLE_UART->ctrl1_bit.uen = TRUE;
}
