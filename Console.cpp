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

#include "Global.h"
#include "Console.h"
#include <stdarg.h>

void Console::printf(const char *format, ...)
{
	char *output;
	va_list args;
	int lenOut;
	va_start(args, format);
	lenOut = vsnprintf(NULL, 0, format, args) + 1;
	va_end(args);
	output = (char *)memMalloc(lenOut);
	va_start(args, format);
	vsprintf(output, format, args);
	va_end(args);
	Print(std::string(output));
	free(output);
}

#define COLOUR(Code) "\x1B["Code"m"
#define COMBINE(C1, C2) C1";"C2
#define LIGHT "1"
#define DARK "2"
#define BLACK "30"
#define RED "31"
#define GREEN "32"
#define BROWN "33"
#define BLUE "34"
#define MAGENTA "35"
#define CYAN "36"
#define WHITE "37"

const char *Console::ColourToString(char IRCColour)
{
	switch (IRCColour)
	{
		case '0':
			return COLOUR(COMBINE(LIGHT, WHITE));
		case '1':
			return COLOUR(COMBINE(DARK, BLACK));
		case '2':
			return COLOUR(COMBINE(LIGHT, RED));
		case '3':
			return COLOUR(BROWN);
		case '4':
			return COLOUR(COMBINE(LIGHT, BROWN));
		case '5':
			return COLOUR(COMBINE(LIGHT, GREEN));
		case '6':
		case '7':
			return COLOUR(GREEN);
		case '8':
			return COLOUR(COMBINE(LIGHT, CYAN));
		case '9':
			return COLOUR(COMBINE(LIGHT, BLUE));
		case ':':
			return COLOUR(BLUE);
		case ';':
			return COLOUR(MAGENTA);
		case '<':
		case '=':
			return COLOUR(COMBINE(LIGHT, MAGENTA));
		case '>':
			return COLOUR(WHITE);
		case '?':
			return COLOUR(COMBINE(DARK, WHITE));
		case '@':
			return COLOUR(COMBINE(DARK, RED));
		case 'A':
			return COLOUR(COMBINE(DARK, BROWN));
		case 'B':
			return COLOUR(BROWN);
		case 'C':
		case 'D':
		case 'E':
			return COLOUR(COMBINE(DARK, GREEN));
		case 'F':
			return COLOUR(COMBINE(DARK, CYAN));
		case 'G':
		case 'H':
			return COLOUR(COMBINE(DARK, BLUE));
		case 'I':
		case 'J':
		case 'K':
			return COLOUR(COMBINE(DARK, MAGENTA));
	}
	return "\x1B[0m";
}

void Console::Print(std::string Output)
{
	size_t pos;
	bool Bold = false, Underline = false, Reverse = false, Colour = false;

	while ((pos = Output.find_first_of("\x02\x03\x0F\x16\x1F")) != std::string::npos)
	{
		switch (Output[pos])
		{
			case 0x02:
				if (Bold == false)
					Output.replace(pos, 1, "\x1B[1m"), Bold = true;
				else
					Output.replace(pos, 1, "\x1B[22m"), Bold = false;
				break;
			case 0x03:
				Output.replace(pos, 2, ColourToString(Output[pos + 1])), Colour = true;
				break;
			case 0x0F:
				Output.replace(pos, 1, "\x1B[0m");
				break;
			case 0x16:
				if (Reverse == false)
					Output.replace(pos, 1, "\x1B[7m"), Reverse = true;
				else
					Output.replace(pos, 1, "\x1B[27m"), Reverse = false;
				break;
			case 0x1F:
				if (Underline == false)
					Output.replace(pos, 1, "\x1B[4m"), Underline = true;
				else
					Output.replace(pos, 1, "\x1B[24m"), Underline = false;
				break;
		}
	}

	if (Colour == true)
		Output.append("\x1B[0;39m"), Colour = false;

	::printf(Output.c_str());
}
