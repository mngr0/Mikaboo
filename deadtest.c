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


#include <uARMconst.h>
#include <uARMtypes.h>
#include <libuarm.h>

#include "nucleus.h"
#include "p2test.h"

static struct tcb_t* printid[8];

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

void ttyNout_thread() {

    uintptr_t payload;
    struct tcb_t* sender;
    unsigned int term;// =  TERM0ADDR + (0x10 *n);
    msgrecv(NULL,&term);
    for (;;) {
        sender = msgrecv(NULL, &payload);
        ttyprintstring(term, (char*) payload);
        msgsend(sender, NULL);
    }
}

static inline void ttyNprint(int n, char* s) {
    msgsend(printid[n], s);
    msgrecv(printid[n], NULL);
}

#define tty0print(s) ttyNprint(0,s)

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


void cs_thread(void) {
    struct tcb_t* sender;
    for (;;) {
        sender = msgrecv(NULL, NULL);
        msgsend(sender, NULL);
        msgrecv(sender, NULL);
    }
}

static state_t tmpstate;
memaddr stackalloc;

void p2(), p3();

struct tcb_t* testt, * p2t, * p3t;
struct tcb_t * p2tt, * p3tt;

struct tcb_t * call_create_process(memaddr function){
    CSIN();
    tmpstate.sp = (stackalloc -= QPAGE);
    CSOUT;
    tmpstate.pc = (memaddr) function;
    return create_process(&tmpstate);
}

void AA(){}

void create_printerN(int n){
    AA();
    tmpstate.sp = (stackalloc -= QPAGE);
    tmpstate.pc = (memaddr) ttyNout_thread;
    tmpstate.cpsr = STATUS_ALL_INT_ENABLE(tmpstate.cpsr);
    printid[n] = create_thread(&tmpstate);
    msgsend(printid[n], TERM0ADDR+ (0x10 * n));
    ttyNprint(n,"terminal open\n");
}


void test(void) {
    ttyprintstring(TERM0ADDR, "PARALLEL TEST!\n");
    STST(&tmpstate);
    stackalloc = (tmpstate.sp + (QPAGE - 1)) & (~(QPAGE - 1));
    int j;
    for(j=0;j<1;j++){
        create_printerN(j);
    }

    testt = get_mythreadid();

    tmpstate.sp = (stackalloc -= QPAGE);
    tmpstate.pc = (memaddr) cs_thread;
    csid = create_process(&tmpstate);
    tty0print("NUCLEUS: critical section thread started\n");

    CSIN();
    tmpstate.sp = (stackalloc -= QPAGE);
    CSOUT;
    tmpstate.pc = (memaddr) test2;
    p2tt = create_thread(&tmpstate);

    CSIN();
    tmpstate.sp = (stackalloc -= QPAGE);
    CSOUT;
    tmpstate.pc = (memaddr) test3;
    p3tt = create_thread(&tmpstate);


    tty0print("dead?");

}
