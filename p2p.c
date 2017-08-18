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



void tty0out_thread(void) {
    uintptr_t payload;
    struct tcb_t* sender;

    for (;;) {
        sender = msgrecv(NULL, &payload);
        ttyprintstring(TERM0ADDR, (char*) payload);
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


void p2(), p3(), p4(), p5(), p6(), p7(), p8();

struct tcb_t* testt, * p2t, * p3t, * p4t, * p5t, * p6t, * p7t, * p8t;
struct tcb_t * p2tt, * p3tt, * p4tt, * p5tt, * p6tt, * p7tt, * p8tt;

static state_t tmpstate;
memaddr stackalloc;

uintptr_t p5sys = 0;
uintptr_t p5send = 0;

struct tcb_t * call_create_process(memaddr function){
    CSIN();
    tmpstate.sp = (stackalloc -= QPAGE);
    CSOUT;
    tmpstate.pc = (memaddr) function;
    return create_process(&tmpstate);
}

void test2(){

   struct tct_b* parent=  msgrecv(NULL, NULL);
    p2t = call_create_process((memaddr)p2);
    msgsend(p2t,SYNCCODE);
    msgrecv(p2t, NULL);
    ttyNprint(1,"p2 completed\n");
    msgsend(parent,SYNCCODE);
    terminate_thread();
}

void test3(){
    struct tct_b* parent= msgrecv(NULL, NULL);
    p3t = call_create_process((memaddr)p3);
    msgrecv(p3t, NULL);
    ttyNprint(2,"p3 completed\n");
    msgsend(parent,SYNCCODE);
    terminate_thread();
}

void test4(){
    struct tct_b* parent= msgrecv(NULL, NULL);
    p4t = call_create_process((memaddr)p4);
    msgsend(p4t, NULL);
    msgrecv(p4t, NULL);
    msgsend(p4t, tmpstate.sp);
    msgrecv(p4t, NULL);
    if (geterrno() == 0)
        panic("p1 wrong errno: recv from p4 should abort, p4 terminated\n");
    else {
        ttyNprint(3,"p4 errno ok\n");
    }
    ttyNprint(3,"p4 completed\n");
    msgsend(parent,SYNCCODE);
    terminate_thread();
}


void test5(){
    struct tct_b* parent= msgrecv(NULL, NULL);
    p5t = call_create_process((memaddr)p5);
    msgsend(p5t, NULL);
    msgrecv(p5t, NULL);

    if (p5sys == 1) ttyNprint(4,"p5a usermode sys passup ok\n");
    else panic("p5a usermode passup error\n");

    if (p5send != 2) ttyNprint(4,"p5a usermode msg priv kill is ok\n");
    else panic("p5a usermode msg priv kill error\n");
    msgsend(parent,SYNCCODE);
    terminate_thread();
}

void test6(){
    struct tct_b* parent= msgrecv(NULL, NULL);
    p6t = call_create_process((memaddr)p6);
    ttyNprint(5,"p6 completed\n");
    msgsend(parent,SYNCCODE);
    terminate_thread();
}

void test7(){
    struct tct_b* parent= msgrecv(NULL, NULL);
    p7t = call_create_process((memaddr)p7);
    ttyNprint(6,"p7 completed\n");
    msgsend(parent,SYNCCODE);
    terminate_thread();
}


void test8(){
    struct tct_b* parent= msgrecv(NULL, NULL);
    p8t = call_create_process((memaddr)p8);
    ttyNprint(7,"p8 completed\n");
    msgsend(parent,SYNCCODE);
    terminate_thread();
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
    for(j=0;j<8;j++){
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
    
    CSIN();
    tmpstate.sp = (stackalloc -= QPAGE);
    CSOUT;
    tmpstate.pc = (memaddr) test4;
    p4tt = create_thread(&tmpstate);

    CSIN();
    tmpstate.sp = (stackalloc -= QPAGE);
    CSOUT;
    tmpstate.pc = (memaddr) test5;
    p5tt = create_thread(&tmpstate);

    CSIN();
    tmpstate.sp = (stackalloc -= QPAGE);
    CSOUT;
    tmpstate.pc = (memaddr) test6;
    p6tt = create_thread(&tmpstate);

    CSIN();
    tmpstate.sp = (stackalloc -= QPAGE);
    CSOUT;
    tmpstate.pc = (memaddr) test7;
    p7tt = create_thread(&tmpstate);

    CSIN();
    tmpstate.sp = (stackalloc -= QPAGE);
    CSOUT;
    tmpstate.pc = (memaddr) test8;
    p8tt = create_thread(&tmpstate);            
    
    msgsend(p2tt,SYNCCODE);
    msgsend(p3tt,SYNCCODE);
    msgsend(p4tt,SYNCCODE);
    msgsend(p5tt,SYNCCODE);
    msgsend(p6tt,SYNCCODE);
    msgsend(p7tt,SYNCCODE);
    msgsend(p8tt,SYNCCODE);

    for(j=0;j<8;j++){
        msgrecv(NULL,NULL);
    }
    
    tty0print("IT'S ALIVE! IT'S ALIVE! THE KERNEL IS ALIVE!\n");
    terminate_process();
}


void p2(void) {
    struct tcb_t* p1t;
    uintptr_t value;
    cputime cpu_t1, cpu_t2;
    int i;

    p1t = msgrecv(NULL, &value);
    ttyNprint(1,"p2 started\n");

    /* test: GET_MYTHREADID GET_PROCESSID GET_PARENTPROCID */
    if (get_mythreadid() != p2t)
        panic("p2 get_mythreadid: wrong pid returned\n");

    
    if (value != SYNCCODE)
        panic("p2 recv: got the wrong value\n");
    if (p1t != p2tt)
        panic("p2 recv: got the wrong sender\n");
    if (get_processid(p1t) != get_parentprocid(get_processid(get_mythreadid())))
        panic("p2 get_parentprocid get_processid error\n");

    /* test: GET_CPUTIME */

    cpu_t1 = getcputime();
    /* delay for several milliseconds */
    for (i = 1; i < LOOPNUM; i++);

    cpu_t2 = getcputime();

    if ((cpu_t2 - cpu_t1) >= MINLOOPTIME)
        ttyNprint(1,"p2 GET_CPUTIME sounds okay\n");
    else
        panic("p2 GETCPUTIME sounds faulty\n");

    msgsend(p1t, NULL);
    msgrecv(p1t, NULL);

    terminate_thread();

    panic("p2 survived TERMINATE_THREAD\n");
}

cputime time1, time2,ptimePRE,ptimePOST,waste[NWAIT];
void p3(void) {
    ttyNprint(2,"p3 started\n");

    
    int i;
    time1 = getTODLO();
    
    for (i = 0; i < NWAIT; i++) {
        ptimePRE=getTODLO();
        waitforclock();
        ptimePOST=getTODLO();
        waste[i]=(ptimePOST - ptimePRE)-PSEUDOCLOCK;
    }
    time2 = getTODLO();

    if ((time2 - time1) < (PSEUDOCLOCK * (NWAIT - 1))) {
        panic("WAITCLOCK too small\n");
    } else if ((time2 - time1) > (PSEUDOCLOCK * (NWAIT + 5))) {
        panic("WAITCLOCK too big\n");
    } else {
        ttyNprint(2,"WAITCLOCK OK\n");
    }

    msgsend(testt, "NULL");

    terminate_process();

    panic("p3 survived TERMINATE_PROCESS\n");
}

void p4(void) {
    
    static int p4inc = 1;
    struct tcb_t* parent;
    struct tcb_t* child;
    state_t p4childstate;
    parent = msgrecv(NULL, NULL);
    switch (p4inc) {
        case 1:
            ttyNprint(3,"first incarnation of p4 starts\n");
            break;
        case 2:
            ttyNprint(3,"second incarnation of p4 starts\n");
            break;
    }
    p4inc++;
    //parent = msgrecv(NULL, NULL);
    msgsend(parent, NULL);

    msgrecv(NULL, NULL);
    /* only the first incarnation reaches this point */

    STST(&p4childstate);
    CSIN();
    p4childstate.sp = (stackalloc -= QPAGE);
    CSOUT;
    p4childstate.pc = (memaddr) p4;

    child = create_process(&p4childstate);
    //msgsend(child, SYNCCODE);
    msgsend(child, NULL);
    msgrecv(child, NULL);

    terminate_process();

    panic("p4 survived TERMINATE_PROCESS\n");
}

void p5a();

void p5p(void) {
    struct tcb_t* sender;
    state_t* state;
    short int should_terminate = 0;
    for (;;) {
        sender = msgrecv(NULL, &state);
        switch (CAUSE_EXCCODE_GET(state->CP15_Cause)) {
            case EXC_RESERVEDINSTR:
                ttyNprint(4,"p5a got RESERVEDINSTR\n");
                should_terminate = 1;
                break;
            default:
                PANIC();
        }
        if (should_terminate) {
            terminate_process();
        } else {
            msgsend(sender, NULL);
        }
    }
}

void p5m(void) {
    struct tcb_t* sender;
    state_t* state;
    for (;;) {
        sender = msgrecv(NULL, &state);
        switch (CAUSE_EXCCODE_GET(state->CP15_Cause)) {
            default:
                ttyNprint(4,"p5 mem error passup okay\n");
                sender->t_s.pc = (memaddr) p5a;
                break;
        }
        msgsend(sender, NULL);
    }
}

void p5s(void) {
    uintptr_t retval;
    struct tcb_t* sender;
    state_t* state;
    for (;;) {
        sender = msgrecv(NULL, &state);
        switch (state->a1) {
            case 42:
                retval = 42;
                break;
            case 3:
                retval = 0;
                break;
            default:
                retval = -1;
                break;
        }
        state->a1 = retval;
        msgsend(sender, NULL);
    }
}


void p5(void) {
    state_t mgrstate;
    uintptr_t retval = 200;

    ttyNprint(4,"p5 started\n");

    STST(&mgrstate);
    msgrecv(NULL, &mgrstate.pc);
    CSIN();
    mgrstate.sp = (stackalloc -= QPAGE);
    CSOUT;
    mgrstate.pc = (memaddr) p5p;
    setpgmmgr(create_thread(&mgrstate));

    CSIN();
    mgrstate.sp = (stackalloc -= QPAGE);
    CSOUT;
    mgrstate.pc = (memaddr) p5m;
    settlbmgr(create_thread(&mgrstate));

    CSIN();
    mgrstate.sp = (stackalloc -= QPAGE);
    CSOUT;
    mgrstate.pc = (memaddr) p5s;

    setsysmgr(create_thread(&mgrstate));

    /* this should me handled by p5s */

    retval = SYSCALL(42, 42, 42, 42);
    if (retval == 42)
        ttyNprint(4,"p5 syscall passup okay\n");
    else
        panic("p5 syscall passup error\n");

    *((memaddr*) BADADDR) = 0;

    panic("p5 survived mem error");
}

void p5a(void) {
    uintptr_t retval = 200;
    /* switch to user mode */
    setSTATUS((getSTATUS() & STATUS_CLEAR_MODE) | STATUS_USER_MODE);

    p5sys = 0;

    retval = SYSCALL(3, 2, 1, 0);
    if (retval == 0) {
        p5sys = 1;
    } else {
        p5sys = 2;
    }

    /* this should fail! */
    msgsend(SSI, NULL);

    p5send = 2;
    //infinite loop: this should not be reaached.
    // The loop is added in order to allow debug in case of failure (manually check p5send)
    for (;;);
}

void p6(void) {
    ttyNprint(5,"p6 started\n");
    *((memaddr*) BADADDR) = 0;

    panic("p6 survived mem error");
}

void p7(void) {

    ttyNprint(6,"p7 started\n");

    SYSCALL(3, 2, 1, 0);

    panic("p7 survived syscall without manager");
}

void p8child(), p8grandchild();

void p8(void) {

    ttyNprint(7,"p8 started\n");
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