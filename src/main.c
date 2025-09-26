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

#define _GNU_SOURCE

#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <pudc/print.h>
#include <pudc/pudc.h>
#include <pudc/compiler.h>

/**
 * Ensure that our current PID and TID are all 1, otherwise halt execution.
 */
void ensure_pid_tid_one(void)
{
	pid_t pid, tid;

	pid = getpid();
	if (pid != 1) {
		pr_error(true, "'pudc' is supposed to be PID 1, but it is PID %d", pid);
	}

	tid = gettid();
	if (tid != 1) {
		pr_error(true, "'pudc' is supposed to be TID 1, but it is TID %d", tid);
	}
}

/**
 * Ensure that the given @path is an existing directory.
 *
 * NOTE: this function will halt execution in case of error.
 */
void ensure_dir(const char *const path)
{
	struct stat st;
	int status = stat(path, &st);

	if (!status) {
		// Nothing to be done!
		if (S_ISDIR(st.st_mode)) {
			pr_debug("directory '%s' found", path);
			return;
		}

		// It exists but it's not a directory, bail out.
		pr_error(true, "'%s' exists and it's not a directory", path);
	}

	pr_error(true, "failed to stat '%s'", path);
}

__noreturn int main(void)
{
	pr_info("starting pudc v%s", PUDC_VERSION);

	/*
	 * Early checks.
	 */
	ensure_pid_tid_one();
	ensure_dir("/run/pudc");

	// TODO
	halt();

	/*
	 * We should never reach this point, and if we do just halt. The second
	 * 'halt' is just to make the compiler happy.
	 */
	pr_error(true, "pudc is no longer accepting requests. Halting execution..", "asd");
	halt();
}
