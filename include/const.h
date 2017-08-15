#ifndef CONST_H
#define CONST_H
//numero massimo
#define MAXPROC 20
#define MAXTHREAD 30
#define MAXMSG 40
//ERR_NUMBER DEFINITI
#define NO_ERR 0
#define ERR_SEND_TO_DEAD 1
#define ERR_RECV_FROM_DEAD 2
//offset del terminale
#define TERM_STATUS_READ   0x00000000
#define TERM_COMMAND_READ   0x00000004
#define TERM_STATUS_WRITE   0x00000008
#define TERM_COMMAND_WRITE   0x0000000C
#define COMMAND_REG_OFFSET 4
#define DEV_FIELD_SIZE (WS*2)//ogni device occupa 4=DEV_REG_SIZE word, ma i registri usati solo due
//Tempi
#define SCHED_TIME_SLICE 5000
#define SCHED_PSEUDO_CLOCK 100000
//valore massimo della richiesta
#define MAX_REQUEST_VALUE 13

#endif
