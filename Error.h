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

#ifndef __Error_H__
#define __Error_H__

#define E_MESSAGE_TOO_SHORT 1
#define E_BAD_COMMAND 2
#define E_ILLEGAL_CHARACTER 3
#define E_TOO_MANY_PARAMETERS 4
#define E_NOT_ENOUGH_PARAMETERS 5
#define E_BAD_REALLOC 6

extern const char *GetNetError();

class IRCMessageError
{
public:
	IRCMessageError(uint32_t errorCode);
	void print();

private:
	uint32_t errorCode;
};

class IRCError
{
public:
	IRCError(const char *netError);
	void print();

private:
	const char *netError;
};

#endif /*__Error_H__*/
