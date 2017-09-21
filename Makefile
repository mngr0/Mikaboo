/*****************************************************************************
 * mikabooq.c Year 2017 v.0.1 Luglio, 15 2017                              *
 * Copyright 2017 Simone Berni, Marco Negrini, Dorotea Trestini              *
 *                                                                           *
 * This file is part of MiKABoO.                                             *
 *                                                                           *
 * MiKABoO is free software; you can redistribute it and/or modify it under  *
 * the terms of the GNU General Public License as published by the Free      *
 * Software Foundation; either version 2 of the License, or (at your option) *
 * any later version.                                                        *
 * This program is distributed in the hope that it will be useful, but       *
 * WITHOUT ANY WARRANTY; without even the implied warranty of                *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General *
 * Public License for more details.                                          *
 * You should have received a copy of the GNU General Public License along   *
 * with this program; if not, write to the Free Software Foundation, Inc.,   *
 * 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA.                  *
 *****************************************************************************/

CC = arm-none-eabi-gcc
CFLAGS = -mcpu=arm7tdmi -c -I /usr/include/uarm/ -I include
LD = arm-none-eabi-ld
EF = elf2uarm -k
LDFLAGS = -T /usr/include/uarm/ldscripts/elf32ltsarm.h.uarmcore.x /usr/include/uarm/crtso.o /usr/include/uarm/libuarm.o
STDOBJECTS =  mikabooq.o  boot.o exceptions.o scheduler.o ssi.o interrupts.o

STDTEST = $(STDOBJECTS) p2test.o
PARTEST = $(STDOBJECTS) p2p.o

all: mikaboo.core

parallel: partest
	$(EF) mikaboo

mikaboo.core: mikaboo
	$(EF) mikaboo

partest: $(PARTEST)
	$(LD) $(LDFLAGS) -o mikaboo $(PARTEST)


mikaboo: $(STDTEST)
	$(LD) $(LDFLAGS) -o mikaboo $(STDTEST)

rebuild: cleanall all

%.o: %.c
	$(CC) $(CFLAGS) $<

clean:
	rm -rf *.o mikaboo

cleanall:
	rm -rf *.o mikaboo mikaboo.core.uarm mikaboo.stab.uarm
