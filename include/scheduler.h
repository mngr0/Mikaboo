#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <listx.h>
#include "nucleus.h"
//#define BUS_INTERVALTIMER 0x10000020
//#define BUS_TIMESCALE 0x10000024
#define SCHED_TIME_SLICE 5000
#define SCHED_PSEUDO_CLOCK 100000

#define device_free 1;
#define device_busy 2;        

struct dev_acc_ctrl{
	struct list_head acc;
	int state;
	devaddr device;
};

void init_dev_ctrl();

struct dev_acc_ctrl disk_queue[8];
struct dev_acc_ctrl tape_queue[8];
struct dev_acc_ctrl ethernet_queue[8];
struct dev_acc_ctrl printer_queue[8]; 
struct dev_acc_ctrl terminal_queue[8];
//#define SET_IT(timer_val) ((*((unsigned int *)BUS_INTERVALTIMER)) = (timer_val * (*(unsigned int *)BUS_TIMESCALE)))

struct list_head readyQueue;

struct list_head waitingQueue;

struct list_head waitForPseudoClockQueue;

struct tcb_t* currentThread;


int threadCount;
int softBlockCount;
//extern unsigned int clock_TOD;
//extern unsigned int slice_TOD;
//extern unsigned int process_TOD;

int isTimer(unsigned int TIMER_TYPE);
void scheduler();

#endif
