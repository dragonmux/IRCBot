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
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdarg.h>
#include <errno.h>

using namespace rSON;

pthread_mutex_t IRCMutex;
sockaddr_in service;

IRC::IRC() : con(-1), RecvThread(0), configRoot(parseJSONFile("server.json"))
{
	pthread_attr_t RecvAttrs;
	pthread_mutexattr_t IRCAttrs;
	hostent *ircHost;
	bool opt = true;

	verifyConfig();
	JSONObject &config = configRoot->asObjectRef();

	service.sin_family = AF_INET;
	service.sin_port = htons(config["port"]->asInt());
	ircHost = gethostbyname(config["server"]->asString());
	if (ircHost == NULL)
		throw new IRCError("Could not look up server!");
	service.sin_addr.s_addr = ((struct in_addr *)(ircHost->h_addr_list[0]))->s_addr;

	con = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
	if (con == -1)
		throw new IRCError(GetNetError());
	setsockopt(con, SOL_SOCKET, SO_KEEPALIVE, (char *)&opt, sizeof(opt));
	opt = false;
	setsockopt(con, SOL_SOCKET, SO_LINGER, (char *)&opt, sizeof(opt));

	if (connect(con, (sockaddr *)&service, sizeof(service)) == -1)
	{
		if (errno != EINPROGRESS)
			throw new IRCError(GetNetError());
		else
			sleep(1);
	}

	pthread_mutexattr_init(&IRCAttrs);
	pthread_mutexattr_settype(&IRCAttrs, PTHREAD_MUTEX_ERRORCHECK);
	pthread_mutex_init(&IRCMutex, &IRCAttrs);
	pthread_mutexattr_destroy(&IRCAttrs);

	pthread_attr_init(&RecvAttrs);
	pthread_attr_setdetachstate(&RecvAttrs, PTHREAD_CREATE_JOINABLE);
	pthread_attr_setscope(&RecvAttrs, PTHREAD_SCOPE_PROCESS);
	pthread_create(&RecvThread, &RecvAttrs, ThreadedRecv, this);
	pthread_attr_destroy(&RecvAttrs);

	nick = config["nick"]->asString();
}

IRC::~IRC()
{
	if (con != -1)
	{
		if (RecvThread != 0)
		{
			vaSend("Quit :%s is going down", nick);
			pthread_cancel(RecvThread);
			pthread_join(RecvThread, NULL);
		}
#if 0
		// this fails when the bot is shut down via "kill -TERM [botPID]"
		shutdown(con, SHUT_RDWR);
		close(con);
#endif
		pthread_mutex_destroy(&IRCMutex);
	}
	delete configRoot;
}

void IRC::verifyConfig()
{
	if (configRoot == NULL || configRoot->getType() != JSON_TYPE_OBJECT)
		throw new IRCError("Invalid config");
	JSONObject &config = configRoot->asObjectRef();
	if (!config.exists("server") || !config.exists("port") || !config.exists("nick") ||
		config["server"]->getType() != JSON_TYPE_STRING || config["port"]->getType() != JSON_TYPE_INT ||
		config["nick"]->getType() != JSON_TYPE_STRING)
		throw new IRCError("Configuration contains errors");
	if ((!config.exists("channel") && !config.exists("channels")) ||
		(config.exists("channel") && config["channel"]->getType() != JSON_TYPE_STRING) ||
		(config.exists("channels") && config["channels"]->getType() != JSON_TYPE_ARRAY))
		throw new IRCError("Configuration contains errors");
	if (config.exists("channels"))
	{
		JSONArray &channels = config["channels"]->asArrayRef();
		for (size_t i = 0; i < channels.size(); i++)
		{
			if (channels[i] == NULL || channels[i]->getType() != JSON_TYPE_STRING)
				throw new IRCError("Configuration contains errors");
		}
	}
	if (config.exists("identity"))
	{
		if (config["identity"]->getType() != JSON_TYPE_OBJECT)
			throw new IRCError("Configuration contains errors");
		else
		{
			JSONObject &ident = config["identity"]->asObjectRef();
			if (!ident.exists("nick") || !ident.exists("password") || !ident.exists("service") ||
				ident["nick"]->getType() != JSON_TYPE_STRING ||
				ident["password"]->getType() != JSON_TYPE_STRING ||
				ident["service"]->getType() != JSON_TYPE_STRING)
				throw new IRCError("Configuration contains errors");
		}
	}
}

void IRC::Connect()
{
	JSONObject &config = configRoot->asObjectRef();

	vaSend("NICK %s", nick);
	if (config.exists("user") && config["user"]->getType() == JSON_TYPE_STRING)
		vaSend("USER %s . . :%s IRC Bot", config["user"]->asString(), nick);
	else
		vaSend("USER %s . . :%s IRC Bot", config["nick"]->asString(), nick);
}

void IRC::JoinChannels()
{
	JSONObject &config = configRoot->asObjectRef();

	if (config.exists("identity"))
	{
		JSONObject &ident = config["identity"]->asObjectRef();
		::con->printf("Identifying with %s as %s\n", ident["service"]->asString(), ident["nick"]->asString());
		vaSend("PRIVMESG %s: identify %s %s", ident["service"]->asString(), ident["nick"]->asString(),
			ident["password"]->asString());
	}

	if (config.exists("channel"))
		vaSend("JOIN %s", config["channel"]->asString());
	else
	{
		JSONArray &channels = config["channels"]->asArrayRef();
		for (size_t i = 0; i < channels.size(); i++)
			vaSend("JOIN %s", channels[i]->asString());
	}
}

void *IRC::ThreadedRecv(void *p_This)
{
	IRC *Self = (IRC *)p_This;

	while (1)
	{
		IRCMessage *msg = NULL;
		try
		{
			msg = IRCMessage::Receive(Self);
			msg->queueCommandProcessing(Self);
		}
		catch (IRCMessageError *e)
		{
			e->print();
			delete msg;
			throw;
		}
		delete msg;
	}

	return 0;
}

char *IRC::safeRealloc(char *m, int s)
{
	void *nM = realloc(m, s);
	if (nM == NULL)
	{
		free(m);
		throw new IRCMessageError(E_BAD_REALLOC);
	}
	return (char *)nM;
}

char *IRC::Recv(int *nBytes)
{
	char c;
	int retBytes = 0;
	char *ret = (char *)malloc(retBytes);
	do
	{
		ret = safeRealloc(ret, retBytes + 1);
		if (recv(con, &c, 1, 0) == -1)
		{
			free(ret);
			throw new IRCError(GetNetError());
		}
		ret[retBytes] = c;
		retBytes++;
	}
	while (c != '\r' && c != '\n');
	*nBytes = retBytes;
	return ret;
}

#define vaMsgToStr(format, varg, expandedMessage) \
{ \
	int lenExpandedMessage; \
	va_list args; \
	va_start(args, varg); \
	lenExpandedMessage = vsnprintf(NULL, 0, format, args); \
	va_end(args); \
	expandedMessage = (char *)malloc(lenExpandedMessage + 1); \
	va_start(args, varg); \
	vsprintf(expandedMessage, format, args); \
	va_end(args); \
}

#define MsgToStr(format, expandedMessage, ...) \
{ \
	int lenExpandedMessage = snprintf(NULL, 0, format, __VA_ARGS__); \
	expandedMessage = (char *)malloc(lenExpandedMessage + 1); \
	sprintf(expandedMessage, format, __VA_ARGS__); \
}

void IRC::vaSend(const char *message, ...)
{
	char *expandedMessage;
	vaMsgToStr(message, message, expandedMessage);
	Send(expandedMessage);
	free(expandedMessage);
}

void IRC::privMsgSend(const char *who, const char *message, ...)
{
	char *preMessage, *expandedMessage;
	MsgToStr("PRIVMSG %s :%s", preMessage, who, message);
	vaMsgToStr(preMessage, message, expandedMessage);
	free(preMessage);
	Send(expandedMessage);
	free(expandedMessage);
}

void IRC::Send(const char *message)
{
	char *realMessage;
	int lenRealMessage = snprintf(NULL, 0, "%s\r\n", message);
	realMessage = (char *)malloc(lenRealMessage + 1);
	sprintf(realMessage, "%s\r\n", message);
	send(con, realMessage, lenRealMessage, 0);
	free(realMessage);
}

const char *IRC::GetNick() const
{
	return nick;
}

IRCMessage::IRCMessage(const char *line, uint32_t LineLength) : Line(line), count(0), lineLength(LineLength),
	Prefix(NULL)
{
	Parameters.clear();
	setPrefix();
	setCommand();
	setParameters();
}

IRCMessage *IRCMessage::Receive(IRC *Connection)
{
	int lineLength = 0;
	char *line = NULL;
	while (line == NULL || lineLength == 0)
	{
		free(line);
		line = Connection->Recv(&lineLength);
		if (lineLength <= 2)
			lineLength = 0;
	}
	return new IRCMessage(line, lineLength);
}

IRCMessage::~IRCMessage()
{
	size_t i;
	free(Prefix);
	for (i = 0; i < Parameters.size(); i++)
		free(Parameters[i]);
	Parameters.clear();
}

bool IRCMessage::isNL(char x)
{
	return (x == '\r' || x == '\n');
}

bool IRCMessage::isWhite(char x)
{
	return x == ' ';
}

bool IRCMessage::isWhiteOrNL(char x)
{
	return isWhite(x) || isNL(x);
}

bool IRCMessage::isAlpha(char x)
{
	x = toupper(x);
	return (x >= 'A' && x <= 'Z');
}

bool IRCMessage::isDigit(char x)
{
	return (x >= '0' && x <= '9');
}

bool IRCMessage::isEOS()
{
	return (count == lineLength - 1 || isNL(Line[count]) == true);
}

int IRCMessage::ctoi(char x)
{
	return x - '0';
}

void IRCMessage::checkCount()
{
	if (count >= lineLength)
		throw new IRCMessageError(E_MESSAGE_TOO_SHORT);
}

void IRCMessage::skipWhite()
{
	while (isWhite(Line[count]) == true)
	{
		count++;
		checkCount();
	}
}

char *IRCMessage::copyString(uint32_t start)
{
	char *ret;
	ret = (char *)malloc(count - start + 1);
	memcpy(ret, &Line[start], count - start);
	ret[count - start] = 0;
	return ret;
}

char *IRCMessage::getStringNoWhite()
{
	// store the start count and find the end of the string
	int startCount = count;
	while (isWhiteOrNL(Line[count]) == false)
	{
		count++;
		checkCount();
	}
	// Store the string and return it
	return copyString(startCount);
}

char *IRCMessage::getStringWhite()
{
	int startCount;
	// Check for the leading colon
	if (Line[count] != ':')
		throw new IRCMessageError(E_ILLEGAL_CHARACTER);
	// Skip the leading colon
	count++;
	// store the start count and find the end of the string
	startCount = count;
	while (isNL(Line[count]) == false)
	{
		count++;
		checkCount();
	}
	// Store the string and return it
	return copyString(startCount);
}

char *IRCMessage::getStringAlpha()
{
	// store the start count and find the end of the string
	int startCount = count;
	while (isWhiteOrNL(Line[count]) == false)
	{
		if (isAlpha(Line[count]) == false)
			throw new IRCMessageError(E_ILLEGAL_CHARACTER);
		count++;
		checkCount();
	}
	// Store the string and return it
	return copyString(startCount);
}

void IRCMessage::setPrefix()
{
	// Is there a prefix?
	if (Line[count] != ':')
		return;
	count++;
	// Yes, so store it
	Prefix = getStringNoWhite();
	// Skip past the whitespace after the prefix to be consistant with what setCommand() expects
	skipWhite();
}

// abuse the C preprocessor to make this nicer!
#define CMDCASE(cmdName, cmdEnum) \
	if (strcmp(cmd, (cmdName)) == 0) \
		Command = (cmdEnum)

void IRCMessage::setStringCommand()
{
	char *cmd = getStringAlpha();
	skipWhite();
	checkCount();

	// convert the command to something in our Enumeration
	CMDCASE("PING", CMD_PING);
	else CMDCASE("NOTICE", CMD_NOTICE);
	else CMDCASE("PRIVMSG", CMD_MESG);
	else CMDCASE("SQUIT", CMD_SQUIT);
	else CMDCASE("MODE", CMD_MODE);
	else CMDCASE("TOPIC", CMD_TOPIC);
	else CMDCASE("NAMES", CMD_NAMES);
	else
		Command = IRC_UNKNOWN;
}

#undef CMDCASE

// abuse the C preprocessor to make this nicer!
#define CMDCASE(cmdNum, cmdEnum) \
	case (cmdNum): \
		Command = (cmdEnum); \
		break

void IRCMessage::setIntCommand()
{
	int cmd;
	// Check the components.. if any of them are bad, bail with E_BAD_COMMAND
	if (isDigit(Line[count]) == false || isDigit(Line[count + 1]) == false ||
		isDigit(Line[count + 2]) == false || isWhite(Line[count + 3]) == false)
		throw new IRCMessageError(E_BAD_COMMAND);

	// convert number and check we haven't gone and used memory that's not ours..
	// no harm if we have, but we do need to throw an exception and reset the client interface
	cmd = (ctoi(Line[count + 0]) * 100) + (ctoi(Line[count + 1]) * 10) + ctoi(Line[count + 2]);
	count += 3;
	skipWhite();
	checkCount();

	// convert the command to something in our Enumeration
	switch (cmd)
	{
		CMDCASE(221, RPL_UMODEIS);
		CMDCASE(324, RPL_CHANNELMODEIS);
		CMDCASE(331, RPL_NOTOPIC);
		CMDCASE(332, RPL_TOPIC);
		CMDCASE(353, RPL_NAMEREPLY);
		CMDCASE(366, RPL_ENDOFNAMES);
		CMDCASE(372, RPL_MOTD);
		CMDCASE(375, RPL_MOTDSTART);
		CMDCASE(376, RPL_MOTDEND);
		default:
			Command = IRC_UNKNOWN;
	}
}

#undef CMDCASE

void IRCMessage::setCommand()
{
	// Identify which command was sent
	if (isAlpha(Line[count]) == true)
		setStringCommand();
	else if (isDigit(Line[count]) == true)
		setIntCommand();
	else
		throw new IRCMessageError(E_BAD_COMMAND);
}

void IRCMessage::setParameters()
{
	// Check there are parameters
	if (isEOS() == true)
		return;

	// Get all non-trailing parameters
	while (Line[count] != ':' && isEOS() == false)
	{
		Parameters.push_back(getStringNoWhite());
		skipWhite();
	}

	// Check for a trailing paramter, and store it if there is one
	if (isEOS() == false)
		Parameters.push_back(getStringWhite());
}

void IRCMessage::sendResponse(IRC *Connection)
{
	if (Command == CMD_PING)
	{
		if (Parameters.size() > 1)
			throw new IRCMessageError(E_TOO_MANY_PARAMETERS);
		else if (Parameters.size() < 1)
			throw new IRCMessageError(E_NOT_ENOUGH_PARAMETERS);
		Connection->vaSend("PONG :%s", Parameters[0]);
	}
	else if (Command == RPL_MOTDEND)
		Connection->JoinChannels();
}

void IRCMessage::queueCommandProcessing(IRC *Connection)
{
	if (Command == CMD_PING || Command == RPL_MOTDEND)
		sendResponse(Connection);
	if (Command == IRC_UNKNOWN)
		con->printf(Line);
	else if (Command != CMD_PING)
		commandQueue.push(new Request(Command, Parameters, Prefix));
}
