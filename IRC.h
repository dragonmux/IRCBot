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

#ifndef __IRC_H__
#define __IRC_H__

#include <inttypes.h>
#include <pthread.h>
#include <vector>
#include <rSON.h>

extern pthread_mutex_t IRCMutex;

typedef enum _IRCCommand
{
	IRC_UNKNOWN,
	// CMD_* - commands
	CMD_PING,
	CMD_NOTICE,
	CMD_MESG,
	CMD_SQUIT,
	CMD_MODE,
	CMD_TOPIC,
	CMD_NAMES,
	CMD_QUIT,
	// RPL_* - replys
	RPL_WELCOME,
	RPL_YOURHOST,
	RPL_CREATED,
	RPL_INFO,
	RPL_BOUNCE,
	RPL_TOPIC,
	RPL_NOTOPIC,
	RPL_MOTDSTART,
	RPL_MOTD,
	RPL_MOTDEND,
	RPL_CHANNELMODEIS,
	RPL_NAMEREPLY,
	RPL_ENDOFNAMES,
	RPL_UMODEIS
} IRCCommand;

class IRC;

class IRCMessage
{
private:
	const char *Line;
	uint32_t count, lineLength;
	char *Prefix;
	IRCCommand Command;
	std::vector<char *> Parameters;

	void setPrefix();
	void setCommand();
	void setStringCommand();
	void setIntCommand();
	void setParameters();
	char *getStringNoWhite();
	char *getStringWhite();
	char *getStringAlpha();

	inline bool isNL(char x);
	inline bool isWhite(char x);
	inline bool isWhiteOrNL(char x);
	inline bool isAlpha(char x);
	inline bool isDigit(char x);
	inline bool isEOS();
	inline int ctoi(char x);
	inline void skipWhite();
	inline void checkCount();
	inline char *copyString(uint32_t start);

	void sendResponse(IRC *Connection);

public:
	static IRCMessage *Receive(IRC *Connection);
	IRCMessage(const char *Line, uint32_t lineLength);
	~IRCMessage();
	void queueCommandProcessing(IRC *Connection);
};

class IRC
{
private:
	int32_t con;
	pthread_t RecvThread;
	rSON::JSONAtom *configRoot;
	const char *nick;

	static void *ThreadedRecv(void *p_This);
	char *safeRealloc(char *m, int s);
	void verifyConfig();

public:
	IRC();
	~IRC();
	void Connect();
	void JoinChannels();
	void Send(const char *message);
	void vaSend(const char *message, ...);
	void privMsgSend(const char *who, const char *message, ...);
	char *Recv(int *nBytes);
	void Quit();
	void SetServer(const char *server);
	const char *GetNick() const;
};

#endif /*__IRC_H__*/
