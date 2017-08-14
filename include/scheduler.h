#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <listx.h>
#include "nucleus.h"
//Tempi dei block
#define SCHED_TIME_SLICE 5000
#define SCHED_PSEUDO_CLOCK 100000

//VARIABILI GLOBALI
int thread_count;
int soft_block_count;
struct tcb_t* current_thread;
unsigned int process_TOD;
unsigned int waiting_TOD;

struct list_head ready_queue;
struct list_head wait_queue;
struct list_head wait_pseudo_clock_queue;
struct list_head device_list[DEV_USED_INTS*(DEV_PER_INT+1)];
//+1 perche i terminali contano doppio, hanno sia tx che rx


//funzioni
void init_dev_ctrl();
int timer(unsigned int TIMER_TYPE);
void set_next_timer();
void scheduler();

#endif
