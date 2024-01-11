/*
 * Copyright 2023, Neutrality.
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include "serial.h"
#include "utils.h"

/*
 * Initialise the serial port.
 *
 * This code comes from seL4 (src/plat/pc99/machine/io.c).
 */
void serial_init(void)
{
	while (!(in8(SERIAL_PORT + 5) & 0x60)) /* wait until not busy */
		;

	out8(SERIAL_PORT + 1, 0x00); /* disable generating interrupts */
	out8(SERIAL_PORT + 3, 0x80); /* line control register: command: set divisor */
	out8(SERIAL_PORT,     0x01); /* set low byte of divisor to 0x01 = 115200 baud */
	out8(SERIAL_PORT + 1, 0x00); /* set high byte of divisor to 0x00 */
	out8(SERIAL_PORT + 3, 0x03); /* line control register: set 8 bit, no parity, 1 stop bit */
	out8(SERIAL_PORT + 4, 0x0b); /* modem control register: set DTR/RTS/OUT2 */

	in8(SERIAL_PORT);     /* clear receiver SERIAL_PORT */
	in8(SERIAL_PORT + 5); /* clear line status SERIAL_PORT */
	in8(SERIAL_PORT + 6); /* clear modem status SERIAL_PORT */
}

/*
 * Write an unsigned 64-bit integer in decimal notation.
 */
static void serial_put_decimal(uint64_t num)
{
	static const char symbols[] = "0123456789";
	char buf[20 + 1];
	int i = sizeof (buf);

	if (!num) {
		serial_putc('0');
		return;
	}

	buf[--i] = '\0';
	while (num > 0 && --i > 0) {
		buf[i] = symbols[num % 10];
		num /= 10;
	}
	serial_puts(&buf[i]);
}

/*
 * Write an unsigned 64-bit integer in hexadecimal notation.
 */
static void serial_put_hex(uint64_t num)
{
	static const char symbols[] = "0123456789abcdef";
	char buf[16 + 1];
	int i = sizeof (buf);

	serial_putc('0');
	serial_putc('x');

	if (!num) {
		serial_putc('0');
		return;
	}

	buf[--i] = '\0';
	while (num > 0 && --i > 0) {
		buf[i] = symbols[num % 16];
		num /= 16;
	}
	serial_puts(&buf[i]);
}

/*
 * Writes a string to the serial port using a very basic format string syntax.
 */
void serial_printf(const char *fmt, ...)
{
	const char **arg = &fmt;
	arg++;

	while (*fmt != '\0') {
		char c;

		if ((c = *fmt++) != '%') {
			serial_putc(c);
			continue;
		}

		if ((c = *fmt++) == '\0') {
			serial_putc('%');
			break;
		}

		switch (c) {
		case '\0':
			serial_putc('%');
			return;

		case 'c':
			serial_putc(*((uint8_t *) arg));
			arg++;
			break;

		case 's':
			serial_puts(*((char **) arg));
			arg++;
			break;

		case 'x':
			serial_put_hex(*((uint64_t *) arg));
			arg++;
			arg++;
			break;

		case 'd':
			serial_put_decimal(*((uint64_t *) arg));
			arg++;
			arg++;
			break;

		default:
			serial_putc(c);
			break;
		}
	}
}
