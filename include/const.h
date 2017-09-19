#ifndef CONST_H
#define CONST_H

#include "nucleus.h"


typedef unsigned int cpu_t;

//numero massimo
#define MAXPROC 20
#define MAXTHREAD 30
#define MAXMSG 500
//ERR_NUMBER DEFINITI
#define NO_ERR 0
#define ERR_SEND_TO_DEAD 1
#define ERR_RECV_FROM_DEAD 2
#define ERR_MSQ_FULL 3
//offset del terminale
#define TERM_STATUS_READ   0x00000000
#define TERM_COMMAND_READ   0x00000004
#define TERM_STATUS_WRITE   0x00000008
#define TERM_COMMAND_WRITE   0x0000000C
//offset degli altri device
#define STATUS_REG_OFFSET 0x00 
#define COMMAND_REG_OFFSET 0x04 //dev_cmd_offset
#define DATA0_REG_OFFSET 0x08 
#define DATA1_REG_OFFSET 0x0C 


#define DEV_FIELD_SIZE (WS*2)//ogni device occupa 4=DEV_REG_SIZE word, ma i registri usati solo due
//Tempi
#define SCHED_TIME_SLICE 5000
#define SCHED_PSEUDO_CLOCK 100000
//valore massimo della richiesta
#define MAX_REQUEST_VALUE 13

//VARIABILI GLOBALI
int thread_count;
int soft_block_count;
struct tcb_t* current_thread;
cpu_t process_TOD;


struct list_head ready_queue;
struct list_head wait_queue;
struct list_head wait_pseudo_clock_queue;
struct list_head device_list[(DEV_USED_INTS+1)*DEV_PER_INT];//+1 perche i terminali contano doppio, hanno sia tx che rx
#endif
