
CC = arm-none-eabi-gcc
CFLAGS = -mcpu=arm7tdmi -c -I /usr/include/uarm/ -I include
LD = arm-none-eabi-ld
LDFLAGS = -T /usr/include/uarm/ldscripts/elf32ltsarm.h.uarmcore.x /usr/include/uarm/crtso.o /usr/include/uarm/libuarm.o

all: mikaboo

mikaboo: p2test.o  mikabooq.o boot.o
	$(LD) $(LDFLAGS) -o mikaboo mikabooq.o p1test.o boot.o
	elf2uarm -k mikaboo

mikabooq.o : mikabooq.c
	$(CC) $(CFLAGS) -o mikabooq.o mikabooq.c

mikabooq.o : boot.c
	$(CC) $(CFLAGS) -o boot.o boot.c

p1test.o : p2test.c 
	$(CC) $(CFLAGS) -o p2test.o p1test.c
	
clean:
	rm -rf *.o mikaboo
cleanall:
	rm -rf *.o mikaboo mikaboo.core.uarm mikaboo.stab.uarm
