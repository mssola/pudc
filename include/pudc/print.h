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

#pragma once

#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>

/**
 * The type of output being delivered.
 *
 * NOTE: internal; use the 'pr_*' macros instead.
 */
enum print_type {
	info,
	error,
	debug,
};

/**
 * Print the given @message into the @stream while prefixing it with the given
 * @type. The @should_halt boolean determines whether execution should halt or
 * not after printing everything.
 *
 * NOTE: internal; use the 'pr_*' macros instead.
 */
void pr_out(bool should_halt, FILE *stream, enum print_type type, const char *const message, ...);

/**
 * Print the given @message to the standard output. This @message is formatted
 * with stdargs and it will be prefixed with the current local time.
 */
#define pr_info(message, ...) pr_out(false, stdout, info, message, ##__VA_ARGS__)

/**
 * Like 'pr_info', but into the stderr. Moreover, the first argument
 * @should_halt determines whether this function should halt
 */
#define pr_error(should_halt, message, ...) \
	pr_out(should_halt, stderr, error, message, ##__VA_ARGS__)

/**
 * Like 'pr_info' but with "debug" as an @id and it only prints something if
 * __DEBUG__ has been defined.
 */
#ifdef __DEBUG__
#define pr_debug(message, ...) pr_out(false, stdout, debug, message, ##__VA_ARGS__)
#else
#define pr_debug(message, ...) (void)(message)
#endif
