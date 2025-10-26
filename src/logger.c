/*
 * This file is part of the MOD project.
 *
 * Copyright (C) 2025 Zoltán Mánya <zltnmanya@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
 
#include <logger.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>

static char log_lines[LOG_LINE_COUNT][LOG_LINE_LEN];
static int log_line_size[LOG_LINE_COUNT];
static int log_line_to_write = 0;
static int log_line_to_send = 0;

static int adv(int line_nr) {
	line_nr++;
	if (line_nr >= LOG_LINE_COUNT) line_nr = 0;
	return line_nr;
}

int log_printf(const char *fmt, ...) {
	//printf("%s(%s)\n",__func__,fmt);
	int ln = log_line_to_write;

	if (log_line_size[ln] != 0) return -1;

	va_list ap;
	va_start(ap, fmt);
	int n = vsnprintf(log_lines[ln], LOG_LINE_LEN, fmt, ap);
	if (n > LOG_LINE_LEN-1) n = LOG_LINE_LEN-1;
	va_end(ap);

	log_line_size[ln] = n;

	log_line_to_write = adv(ln);

	return n;
}

int log_consume(char *buf, int max_len) {
  int buf_len = 0;

	//printf("%s\n",__func__);
	int buffer_left = max_len;
	while (log_line_size[log_line_to_send] > 0) {
		if (log_line_size[log_line_to_send] > buffer_left) break;

    memcpy(buf+buf_len, log_lines[log_line_to_send], log_line_size[log_line_to_send]);
    buf_len += log_line_size[log_line_to_send];

		// append to endpoint buffer
		buffer_left -= log_line_size[log_line_to_send];
		//printf("appending: \"%s\" -- size:%d -- buffer_left:%d\n", log_lines[log_line_to_send], log_line_size[log_line_to_send], buffer_left);
		log_line_size[log_line_to_send] = 0;
		log_line_to_send=adv(log_line_to_send);
	}

  return buf_len;
}
