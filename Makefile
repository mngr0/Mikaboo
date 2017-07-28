
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
