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


build: $(OBJECTS) $(DEPH) $(EXEC).o
	$(CCT) $(TFLAGS) $(OBJECTS) $(EXEC).o -o release/kernel
           
rebuild: clean build link

%.o: %.c
	$(CC) $(CFLAGS) $< 

link: 
	umps2-elf2umps -k release/kernel

clean:
	rm -rf *.o mikaboo

cleanall:
	rm -rf *.o mikaboo mikaboo.core.uarm mikaboo.stab.uarm
