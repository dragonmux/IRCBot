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
#include "Error.h"
#include <errno.h>

const char *GetNetError()
{
	switch (errno)
	{
		case EADDRINUSE:
			return "Address already in use";
		case EADDRNOTAVAIL:
			return "Cannot assign requested address";
		case EAFNOSUPPORT:
			return "Address family not supported";
		case EALREADY:
			return "Socket already in use";
		case EBADF:
			return "Socket descriptor is bad";
		case ECONNREFUSED:
			return "Connection refused";
		case EFAULT:
			return "Bad address";
		case EINPROGRESS:
			return "Operation in progress";
		case EINTR:
			return "Timeout";
		case EINVAL:
			return "Invalid argument";
		case EISCONN:
			return "Socket already connected";
		case ENETUNREACH:
			return "Network unreachable";
		case ENOTSOCK:
			return "Socket operation on non-socket";
		case ETIMEDOUT:
			return "Connection timed out";
		case ENOTCONN:
			return "Socket is not connected";
		case EHOSTUNREACH:
			return "No route to host";
		case EPIPE:
			return "Broken pipe";
#ifdef ECONNRESET
		case ECONNRESET:
			return "Connection reset by peer";
#endif
#ifdef EACCES
		case EACCES:
			return "Permission denied";
#endif
#ifdef EMFILE
		case EMFILE:
			return "Too many open files";
#endif
		case 0:
			return NULL;
		default:
			return "Unforseen error %d";

	}
}

IRCMessageError::IRCMessageError(uint32_t Error) : errorCode(Error)
{
}

#define ERRORCASE(errNo, errText) \
	case (errNo): \
		con->printf((errText)); \
		break

void IRCMessageError::print()
{
	con->printf("IRCMessageError: ");
	switch (errorCode)
	{
		ERRORCASE(E_MESSAGE_TOO_SHORT, "Message received was too short");
		ERRORCASE(E_BAD_COMMAND, "Message received did not contain a valid command");
		ERRORCASE(E_ILLEGAL_CHARACTER, "Message received contained an illegal character");
		ERRORCASE(E_TOO_MANY_PARAMETERS, "Too many parameters for command");
		ERRORCASE(E_NOT_ENOUGH_PARAMETERS, "Not enough parameters for command");
		ERRORCASE(E_BAD_REALLOC, "Could not reallocate memory - out of memory");
		default:
			con->printf("Unknown error occured");
	}
	if (errorCode != E_BAD_REALLOC)
		con->printf(", resetting connection..\n");
	else
	{
		con->printf(", exiting!\n");
		exit(-1);
	}
}

#undef ERRORCASE

IRCError::IRCError(const char *NetError) : netError(NetError)
{
}

void IRCError::print()
{
	con->printf("I-Bet net error: %s\n", netError);
}
