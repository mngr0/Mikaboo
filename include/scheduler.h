#ifndef SCHEDULER_H
#define SCHEDULER_H


struct list_head readyQueue;

struct list_head waitingQueue;

struct tcb_t* currentProcess;
int processCount =0;

extern unsigned int slice_TOD;
extern unsigned int process_TOD;

int isTimer(unsigned int TIMER_TYPE);
void scheduler();

#endif