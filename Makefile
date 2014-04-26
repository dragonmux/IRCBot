# This file is part of IRCBot
# Copyright © 2014 Rachel Mant (dx-mon@users.sourceforge.net)
#
# IRCBot is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# IRCBot is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

include Makefile.inc

PKG_CONFIG_PKGS = rSON
CFLAGS_EXTRA = $(shell pkg-config --cflags $(PKG_CONFIG_PKGS)) -pthread
CFLAGS = $(OPTIM_FLAGS) -Wall -Wextra $(CFLAGS_EXTRA) -c -o $*.o
LIBS = $(shell pkg-config --libs $(PKG_CONFIG_PKGS)) -pthread -lstdc++
LFLAGS = $(O) $(LIBS) -o $(EXE)

PREFIX ?= /usr
LIBDIR ?= $(PREFIX)/lib

export PREFIX LIBDIR

O = String.o Console.o Error.o Request.o IRC.o Server.o
EXE = IRCBot

default: all

all: $(EXE)

$(EXE): rSON $(O)
	$(call run-cmd,ccld,$(LFLAGS))
	$(call debug-strip,$(EXE))

rSON:
	$(call run-cmd,rSONbuilder)

.cpp.o:
	$(call run-cmd,cxx,$(CFLAGS) $<)

clean:
	$(call run-cmd,rm,$(O))

.PHONY: default all clean .cpp.o rSON
