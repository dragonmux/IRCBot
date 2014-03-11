/*
 * This file is part of IRCBot
 * Copyright © 2014 Rachel Mant (dx-mon@users.sourceforge.net)
 *
 * IRCBot is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * IRCBot is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include "String.h"

#define die(...) \
{ \
	printf(__VA_ARGS__); \
	exit(1); \
}

void *memMalloc(size_t size)
{
	void *ret = malloc(size);
	if (ret == NULL)
		die("**** Future Consciousness Fatal ****\nCould not allocate enough memory!\n**** Future Consciousness Fatal ****");
	memset(ret, 0, size);
	return ret;
}

char *strFormatString(const char *format, ...)
{
	int len;
	char *ret;
	va_list args;
	va_start(args, format);
	len = vsnprintf(NULL, 0, format, args);
	va_end(args);
	ret = (char *)memMalloc(len + 1);
	va_start(args, format);
	vsprintf(ret, format, args);
	va_end(args);
	return ret;
}
