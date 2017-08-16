CC = arm-none-eabi-gcc
CFLAGS = -mcpu=arm7tdmi -c -I /usr/include/uarm/ -I include
LD = arm-none-eabi-ld
EF = elf2uarm -k
LDFLAGS = -T /usr/include/uarm/ldscripts/elf32ltsarm.h.uarmcore.x /usr/include/uarm/crtso.o /usr/include/uarm/libuarm.o
OBJECTS =  mikabooq.o p2test.o  boot.o exceptions.o scheduler.o ssi.o interrupts.o 

all: mikaboo.core

mikaboo.core: mikaboo
	$(EF) mikaboo

mikaboo: $(OBJECTS)
	$(LD) $(LDFLAGS) -o mikaboo $(OBJECTS)

mikabooq.o : mikabooq.c
	$(CC) $(CFLAGS) -o mikabooq.o mikabooq.c

ssi.o : ssi.c
	$(CC) $(CFLAGS) -o ssi.o ssi.c

interrupts.o : interrupts.c
	$(CC) $(CFLAGS) -o interrupts.o interrupts.c

scheduler.o : scheduler.c
	$(CC) $(CFLAGS) -o scheduler.o scheduler.c

exceptions.o : exceptions.c
	$(CC) $(CFLAGS) -o exceptions.o exceptions.c

boot.o : boot.c
	$(CC) $(CFLAGS) -o boot.o boot.c

p2test.o : p2p.c
	$(CC) $(CFLAGS) -o p2test.o p2p.c

clean:
	rm -rf *.o mikaboo
cleanall:
	rm -rf *.o mikaboo mikaboo.core.uarm mikaboo.stab.uarm
