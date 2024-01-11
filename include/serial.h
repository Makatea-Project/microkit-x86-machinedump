/*
 * Copyright 2023, Neutrality.
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include "options.h"
#include "utils.h"

/**
 * Output a single character to the serial port.
 */
static inline void serial_putc(uint8_t ch)
{
	while ((in8(options.serial_port + 5) & 0x20) == 0)
		;
	out8(options.serial_port, ch);
}

/**
 * Output a null-terminated string to the serial port. Note that newlines "\n"
 * are transparently converted to "\r\n".
 */
static inline void serial_puts(const char *s)
{
	while (*s) {
		if (*s == '\n')
			serial_putc('\r');
		serial_putc(*s++);
	}
}

/**
 * Initialise the serial port.
 */
extern void serial_init(void);

/**
 * Writes a string to the serial port using a very basic format string-like
 * syntax.
 */
extern void serial_printf(const char *fmt, ...);
