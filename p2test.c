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


static state_t tmpstate;
memaddr stackalloc;

void test(void) {
    char t= 'n';
    char *s=&t;
    memaddr * base;
    base = (memaddr *) (TERM0ADDR);
    *(base) = 2 | (((memaddr) *s) << 8);

    ttyprintstring(TERM0ADDR, "NUCLEUS TEST: starting...\n");

}

#define MINLOOPTIME             10000
#define LOOPNUM                 10000
