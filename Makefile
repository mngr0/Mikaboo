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
