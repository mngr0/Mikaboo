#ifndef P2TEST_H
#define P2TEST_H

void test();

#define TERM0ADDR               0x24C
#define MINLOOPTIME             100
#define LOOPNUM                 100
#define SYNCCODE 0x01000010
#define QPAGE FRAME_SIZE
#define CSOUT msgsend(csid, NULL)
#define PSEUDOCLOCK 100000
#define NWAIT 10
#define BADADDR 0xFFFFFFFF
#define NGRANDCHILDREN 3

#endif