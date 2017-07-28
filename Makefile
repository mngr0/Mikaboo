
/*****************************************************************************
 * mikabooq.c Year 2017 v.0.1 Febbraio, 04 2017                              *
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
CFLAGS = -mcpu=arm7tdmi -c -I /usr/include/uarm/ -I lib 
LD = arm-none-eabi-ld
LDFLAGS = -T /usr/include/uarm/ldscripts/elf32ltsarm.h.uarmcore.x /usr/include/uarm/crtso.o /usr/include/uarm/libuarm.o

all: mikaboo

mikaboo: p1test.o  mikabooq.o
	$(LD) $(LDFLAGS) -o mikaboo mikabooq.o p1test.o
	elf2uarm -k mikaboo

mikabooq.o : mikabooq.c
	$(CC) $(CFLAGS) -o mikabooq.o mikabooq.c

p1test.o : p1test.c 
	$(CC) $(CFLAGS) -o p1test.o p1test.c
	
clean:
	rm -rf *.o mikaboo
cleanall:
	rm -rf *.o mikaboo mikaboo.core.uarm mikaboo.stab.uarm
