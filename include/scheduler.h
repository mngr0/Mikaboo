#ifndef SCHEDULER_H
#define SCHEDULER_H
//funzioni

int timer(unsigned int TIMER_TYPE);
void set_next_timer();
void scheduler();
int is_time_slice();
void set_pseudo_clock(unsigned int TODLO,int time_until_slice);
extern unsigned int slice_TOD;
extern unsigned int last_TOD;
#endif
