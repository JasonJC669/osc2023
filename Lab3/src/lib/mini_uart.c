#include "utils.h"
#include "peripherals/mini_uart.h"
#include "peripherals/gpio.h"

void uart_send(char c)
{
	while (1)
	{
		if (get32(AUX_MU_LSR_REG) & 0x20)
			break;
	}

	if (c != '\x7f')
		put32(AUX_MU_IO_REG, c);

	if (c == '\n')
		uart_send('\r');
}

char uart_recv(void)
{
	char r;
	while (1)
	{
		if (get32(AUX_MU_LSR_REG) & 0x01)
			break;
	}
	r = get32(AUX_MU_IO_REG);
	return r == '\r' ? '\n' : r;
}

void uart_send_string(char *str)
{
	while (*str)
		uart_send(*str++);
}

void uart_send_space(int size)
{
	while (size--)
		uart_send(' ');
}

void uart_send_string_of_size(char *str, int size)
{
	while (size--)
		uart_send(*str++);
}

/**
 * Display a binary value in hexadecimal
 */
void uart_hex(unsigned int d)
{
	unsigned int n;
	int c;
	for (c = 28; c >= 0; c -= 4)
	{
		// get highest tetrad
		n = (d >> c) & 0xF;
		// 0-9 => '0'-'9', 10-15 => 'A'-'F'
		n += n > 9 ? 0x37 : 0x30;
		uart_send(n);
	}
}

void uart_init(void)
{
	unsigned int selector;

	selector = get32(GPFSEL1);
	selector &= ~(7 << 12); // clean gpio14
	selector |= 2 << 12;	// set alt5 for gpio14
	selector &= ~(7 << 15); // clean gpio15
	selector |= 2 << 15;	// set alt5 for gpio15
	put32(GPFSEL1, selector);

	put32(GPPUD, 0);
	delay(150); // spec
	put32(GPPUDCLK0, (1 << 14) | (1 << 15));
	delay(150);
	put32(GPPUDCLK0, 0);

	put32(AUX_ENABLES, 1);		 // Enable mini uart (this also enables access to its registers)
	put32(AUX_MU_CNTL_REG, 0);	 // Disable auto flow control and disable receiver and transmitter (for now)
	put32(AUX_MU_IER_REG, 0);	 // Disable receive and transmit interrupts
	put32(AUX_MU_LCR_REG, 3);	 // Enable 8 bit mode
	put32(AUX_MU_MCR_REG, 0);	 // Set RTS line to be always high
	put32(AUX_MU_BAUD_REG, 270); // Set baud rate to 115200

	put32(AUX_MU_CNTL_REG, 3); // Finally, enable transmitter and receiver
}

void _putchar(char character)
{
	uart_send(character);
}
