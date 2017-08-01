#ifndef SCHEDULER_H
#define SCHEDULER_H

extern unsigned int slice_TOD;
extern unsigned int process_TOD;

int isTimer(unsigned int TIMER_TYPE);
void scheduler();

#endif