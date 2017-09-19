#ifndef INTERRUPTS_H
#define INTERRUPTS_H
//funzioni

void int_handler();
int get_priority_dev(memaddr* line);
void timer_handler();
void terminal_handler();
void device_handler(int dev_type);
void ack(int dev_type, int dev_numb, unsigned int status, memaddr *command_reg);

extern state_t *int_old ;
#endif
