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

#ifndef __Request_H__
#define __Request_H__
#include "IRC.h"
#include <string>

#define USER_NORMAL 1
#define USER_VOICED 2
#define USER_OPERATOR 4

class Request
{
private:
	char *Prefix;
	IRCCommand Command;
	std::vector<char *> Parameters;
	std::vector<std::string> Trailing;
	IRC *Connection;

	void splitTrailing();
	void handleMesg(IRC *Connection);
	void botCommand();
	void publicCommand();
	void parseNames();

public:
	Request(IRCCommand cmd, std::vector<char *> parameters, char *prefix);
	~Request();
	void process(IRC *Connection);
};

#endif /*__Request_H__*/
