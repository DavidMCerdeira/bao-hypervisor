#include <drivers/16550_uart.h>

void uart_disable(volatile ti_16650_Uart *ptr_uart) {}

void uart_enable(volatile ti_16650_Uart *ptr_uart) {}

void uart_init(volatile ti_16650_Uart *ptr_uart)
{
    console_16550_core_init(ptr_uart, 24000000, 1500000);
}

int uart_getc(volatile ti_16650_Uart *ptr_uart)
{
    return console_16550_getc(ptr_uart);
}

void uart_putc(volatile ti_16650_Uart *ptr_uart, char c)
{
    console_16550_putc(c, ptr_uart);
}

void uart_puts(volatile ti_16650_Uart *ptr_uart, const char *s)
{
    while (*s) {
        uart_putc(ptr_uart, *s++);
    }
}
