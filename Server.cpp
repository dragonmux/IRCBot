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
#include "Error.h"
#include <time.h>
#include <signal.h>

ThreadedQueue<Request *> commandQueue;
Console *con;
bool shutdown = false;

void QuitFn(int)
{
	shutdown = true;
	commandQueue.signalItems();
}

int main(int argc, char **argv)
{
	IRC *IRCCon = NULL;

	signal(SIGTERM, QuitFn);
	signal(SIGQUIT, QuitFn);
	signal(SIGHUP, QuitFn);
	con = new Console();

	// Additional startup code here

	while (shutdown == false)
	{
		try
		{
			IRCCon = new IRC();
		}
		catch (IRCError *e)
		{
			e->print();
			delete e;
			delete IRCCon;
			sleep(2);
			continue;
		}
		catch (rSON::JSONParserError &e)
		{
			fprintf(stderr, "%s\n", e.error());
			delete IRCCon;
			delete con;
			return 1;
		}
		sleep(1);
		try
		{
			IRCCon->Connect();

			while (shutdown == false)
			{
				commandQueue.waitItems();
				if (shutdown == true)
					break;
				while (commandQueue.size() != 0)
				{
					Request *cmd = commandQueue.front();
					commandQueue.pop();
					cmd->process(IRCCon);
					delete cmd;
				}
			}

			while (commandQueue.size() != 0)
			{
				Request *cmd = commandQueue.front();
				commandQueue.pop();
				delete cmd;
			}
		}
		catch (IRCError *e)
		{
			e->print();
			delete e;
		}
		catch (IRCMessageError *e)
		{
			// should already be printed by now, so don't repeat
			delete e;
		}
		delete IRCCon;
	}

	delete con;
	// Additional shutdown code here
	return 0;
}
