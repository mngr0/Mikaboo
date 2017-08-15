void AA(){}
void AB(){}
void AC(){}
void AD(){}
void AE(){}
void AF(){}
void AG(){}
/*
 * Copyright (C) 2017 Renzo Davoli
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <uARMconst.h>
#include <uARMtypes.h>
#include <libuarm.h>

#include "nucleus.h"

#define QPAGE FRAME_SIZE
#define TERM0ADDR               0x24C

static struct tcb_t* printid;

static void ttyprintstring(devaddr device, char* s) {
    uintptr_t status;

    for (; *s; s++) {
        status = do_terminal_io(device, DEV_TTRS_C_TRSMCHAR | (*s << 8));
        switch (status & 0xff) {
            case DEV_S_READY:
            case DEV_TTRS_S_CHARTRSM:
                break;
            default:
                return;
        }
    }
}

void tty0out_thread(void) {
    uintptr_t payload;
    struct tcb_t* sender;

    for (;;) {
        sender = msgrecv(NULL, &payload);
        ttyprintstring(TERM0ADDR, (char*) payload);
        msgsend(sender, NULL);
    }
}

static inline void tty0print(char* s) {
    msgsend(printid, s);
    msgrecv(printid, NULL);
}

static inline void panic(char* s) {
    tty0print("!!! PANIC: ");
    tty0print(s);
    PANIC();
}

static struct tcb_t* csid;

static inline void CSIN() {
    msgsend(csid, NULL);
    msgrecv(csid, NULL);
}

#define CSOUT msgsend(csid, NULL)

void cs_thread(void) {
    struct tcb_t* sender;
    for (;;) {
        sender = msgrecv(NULL, NULL);
        msgsend(sender, NULL);
        msgrecv(sender, NULL);
    }
}

#define SYNCCODE 0x01000010

void p2(), p3(), p4(), p5(), p6(), p7(), p8();

struct tcb_t* testt, * p2t, * p3t, * p4t, * p5t, * p6t, * p7t, * p8t;

static state_t tmpstate;
memaddr stackalloc;

uintptr_t p5sys = 0;
uintptr_t p5send = 0;

void test(void) {
    
  //  ttyprintstring(TERM0ADDR, "NUCLEUS TEST: starting...\n");
    STST(&tmpstate);
    stackalloc = (tmpstate.sp + (QPAGE - 1)) & (~(QPAGE - 1));
    tmpstate.sp = (stackalloc -= QPAGE);
    tmpstate.pc = (memaddr) tty0out_thread;
    tmpstate.cpsr = STATUS_ALL_INT_ENABLE(tmpstate.cpsr);
    printid = create_thread(&tmpstate);
  //  tty0print("NUCLEUS: first msg printed by tty0out_thread\n");
    testt = get_mythreadid();

    tmpstate.sp = (stackalloc -= QPAGE);
    tmpstate.pc = (memaddr) cs_thread;
    csid = create_process(&tmpstate);
  //  tty0print("NUCLEUS: critical section thread started\n");
    CSIN();
    tmpstate.sp = (stackalloc -= QPAGE);
    CSOUT;
    tmpstate.pc = (memaddr) p2;
    p2t = create_process(&tmpstate);
    msgsend(p2t, SYNCCODE);
    msgrecv(p2t, NULL);

    tty0print("p2 completed\n");

    CSIN();
    tmpstate.sp = (stackalloc -= QPAGE);
    CSOUT;
    tmpstate.pc = (memaddr) p6;
    p6t = create_process(&tmpstate);
    msgrecv(p6t, NULL);
    tty0print("p6 completed\n");

    CSIN();
    tmpstate.sp = (stackalloc -= QPAGE);
    CSOUT;
    tmpstate.pc = (memaddr) p7;
    p7t = create_process(&tmpstate);
    msgrecv(p7t, NULL);
    tty0print("p7 completed\n");
    //check total number of thread

    CSIN();
    tmpstate.sp = (stackalloc -= QPAGE);
    CSOUT;
    tmpstate.pc = (memaddr) p8;
    p8t = create_process(&tmpstate);
    msgrecv(p8t, NULL);
    tty0print("p8 completed\n");

    tty0print("IT'S ALIVE! IT'S ALIVE! THE KERNEL IS ALIVE!\n");
    HALT();
}

#define MINLOOPTIME             100
#define LOOPNUM                 100

void p2(void) {
    struct tcb_t* p1t;
    uintptr_t value;
    cputime cpu_t1, cpu_t2;
    int i;

    tty0print("p2 started\n");

    /* test: GET_MYTHREADID GET_PROCESSID GET_PARENTPROCID */
    if (get_mythreadid() != p2t)
        panic("p2 get_mythreadid: wrong pid returned\n");

    p1t = msgrecv(NULL, &value);
    if (value != SYNCCODE)
        panic("p2 recv: got the wrong value\n");
    if (p1t != testt)
        panic("p2 recv: got the wrong sender\n");
    if (get_processid(p1t) != get_parentprocid(get_processid(get_mythreadid())))
        panic("p2 get_parentprocid get_processid error\n");

    /* test: GET_CPUTIME */

    cpu_t1 = getcputime();
    /* delay for several milliseconds */
    for (i = 1; i < LOOPNUM; i++);

    cpu_t2 = getcputime();

    if ((cpu_t2 - cpu_t1) >= MINLOOPTIME)
        tty0print("p2 GET_CPUTIME sounds okay\n");
    else
        panic("p2 GETCPUTIME sounds faulty\n");

    msgsend(p1t, NULL);
    msgrecv(p1t, NULL);

    terminate_thread();

    panic("p2 survived TERMINATE_THREAD\n");
}

#define BADADDR 0xFFFFFFFF

    
void p6(void) {
    tty0print("p6 started\n");
    *((memaddr*) BADADDR) = 0;
    panic("p6 survived mem error");
}

void p7(void) {
    tty0print("p7 started\n");

    SYSCALL(3, 2, 1, 0);

    panic("p7 survived syscall without manager");
}

void p8child(), p8grandchild();

void p8(void) {
    tty0print("p8 started\n");
    state_t childstate;
    struct tcb_t* pid = get_mythreadid();
    STST(&childstate);
    childstate.pc = (memaddr) p8child;
    CSIN();
    childstate.sp = (stackalloc -= QPAGE);
    CSOUT;
    msgsend(create_process(&childstate), pid);
    CSIN();
    childstate.sp = (stackalloc -= QPAGE);
    CSOUT;
    msgsend(create_process(&childstate), pid);
    msgrecv(NULL, NULL);
    msgrecv(NULL, NULL);
    msgrecv(NULL, NULL);
    msgrecv(NULL, NULL);
    msgrecv(NULL, NULL);
    msgrecv(NULL, NULL);

    terminate_process();

    panic("p8 survived TERMINATE_PROCESS\n");
}

#define NGRANDCHILDREN 3

void p8child() {
    int i;
    state_t childstate;
    struct tcb_t* ppid;
    msgrecv(NULL, &ppid);
    for (i = 0; i < NGRANDCHILDREN; i++) {
        STST(&childstate);
        childstate.pc = (memaddr) p8grandchild;
        CSIN();
        childstate.sp = (stackalloc -= QPAGE);
        CSOUT;
        msgsend(create_process(&childstate), ppid);
    }
    msgrecv(NULL, NULL);

    panic("p8child survived parent termination\n");
}

void p8grandchild() {
    struct tcb_t* gpid;
    msgrecv(NULL, &gpid);
    msgsend(gpid, NULL);
    msgrecv(NULL, NULL);

    panic("p8grandchild survived grandparent termination\n");
}