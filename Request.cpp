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
#include "IRC.h"
#include "Request.h"
#include "Error.h"

Request::Request(IRCCommand cmd, std::vector<char *> parameters, char *prefix) : Command(cmd)
{
	uint32_t i;
	Parameters.clear();
	Prefix = (prefix == NULL ? NULL : strdup(prefix));
	for (i = 0; i < parameters.size(); i++)
		Parameters.push_back(strdup(parameters[i]));
}

Request::~Request()
{
	uint32_t i;
	for (i = 0; i < Parameters.size(); i++)
		free(Parameters[i]);
}

void Request::splitTrailing()
{
	size_t start, end;
	std::string param = Parameters[Parameters.size() - 1];
	Trailing.clear();
	for (start = 0, end = 0; end < param.size(); end++)
	{
		if (param[end] == ' ')
		{
			Trailing.push_back(param.substr(start, end - start));
			start = end + 1;
		}
		else if (end + 1 == param.size())
			Trailing.push_back(param.substr(start));
	}
}

// void do<X>()

void Request::parseNames()
{
	uint32_t i;
	splitTrailing();
	for (i = 1; i < Trailing.size(); i++)
	{
		uint8_t flags = USER_NORMAL;
		if (Trailing[i][0] == '+')
			flags |= USER_VOICED;
		else if (Trailing[i][0] == '@')
			flags |= USER_OPERATOR;
		if (flags != USER_NORMAL)
			Trailing[i].erase(0, 1);
	}
}

#define CMDCASE(cmd, idx, function) \
	if (strcasecmp(Trailing[idx].c_str(), cmd) == 0) \
		function

void Request::botCommand()
{
/*	if (Trailing.size() > 2)
	{
	}*/
}

void Request::publicCommand()
{
/*	if (Trailing.size() > 2)
	{
	}*/
}

#undef CMDCASE
#define CMDCASE(cmd, function) \
	if (strcasecmp(Parameters[0], cmd) == 0) \
		function

void Request::handleMesg(IRC *con)
{
	splitTrailing();
	Connection = con;
	if (Trailing.size() > 1)
	{
/*		if (strcmp(Parameters[0], channel) == 0)
		{
			CMDCASE(nick, 0, publicCommand());
		}
		else if (strcmp(Parameters[0], nick) == 0)
			botCommand();*/
	}
}

#undef CMDCASE

void Request::process(IRC *Connection)
{
	uint32_t i;
	switch (Command)
	{
		case RPL_MOTDSTART:
		case RPL_MOTD:
		case RPL_MOTDEND:
			con->printf("%i: %s\n", Parameters.size(), Parameters[Parameters.size() - 1]);
			break;
		case CMD_NOTICE:
			con->printf("%s => %s\n", Parameters[0], Parameters[1]);
			break;
		case CMD_MESG:
			con->printf("%s: <%s> %s\n", Prefix, Parameters[0], Parameters[1]);
			handleMesg(Connection);
			break;
		case CMD_MODE:
			con->printf("Mode change for %s => ", Parameters[0]);
			for (i = 1; i < Parameters.size(); i++)
				con->printf("%s ", Parameters[i]);
			con->printf("\n");
			break;
		case RPL_TOPIC:
			con->printf("Topic for channel %s: %s\n", Parameters[1], Parameters[2]);
			break;
		case RPL_NAMEREPLY:
			con->printf("People on channel %s: %s\n", Parameters[Parameters.size() - 2], Parameters[Parameters.size() - 1]);
			parseNames();
			break;
		default:
			for (i = 0; i < Parameters.size(); i++)
				con->printf("%s ", Parameters[i]);
			con->printf("\n");
	}
}
