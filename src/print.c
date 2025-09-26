// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2024-Ω Miquel Sabaté Solà <mssola@mssola.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <stdarg.h>
#include <stdbool.h>
#include <time.h>
#include <errno.h>
#include <string.h>

#include <pudc/print.h>
#include <pudc/pudc.h>

/**
 * Prints the current local time into the given @stream. If fetching the current
 * local time fails, then it does nothing.
 */
void pr_time(FILE *stream)
{
	time_t current_time = time(NULL);
	struct tm *tm_info = NULL;
	char buffer[32];

	if (current_time != (time_t)-1) {
		tm_info = localtime(&current_time);
		strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", tm_info);
		fprintf(stream, "[%s] ", buffer);
		fflush(stream);
	}
}

void pr_out(bool should_halt, FILE *stream, enum print_type type, const char *const message, ...)
{
	va_list args;

	pr_time(stream);

	fprintf(stream, "pudc (");
	fflush(stream);

	switch (type) {
	case debug:
		fprintf(stream, "debug");
		break;
	case error:
		fprintf(stream, "error");
		break;
	case info:
	default:
		fprintf(stream, "info");
		break;
	};
	fflush(stream);

	fprintf(stream, "): ");
	fflush(stream);

	va_start(args, message);
	vfprintf(stream, message, args);
	fflush(stream);
	va_end(args);

	/*
	 * 'errno' is quite unreliable, so only deal with it if this was called via
	 * 'pr_error'.
	 */
	if (type == error) {
		fprintf(stream, ": ");
		fflush(stream);
		perror(NULL);
		fflush(stream);
	} else {
		fprintf(stream, ".\n");
		fflush(stream);
	}

	if (should_halt) {
		halt();
	}
}
