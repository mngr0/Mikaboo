#ifndef INTERRUPTS_H
#define INTERRUPTS_H
#include "nucleus.h"
//offset del terminale
#define TERM_STATUS_READ   0x00000000
#define TERM_COMMAND_READ   0x00000004
#define TERM_STATUS_WRITE   0x00000008
#define TERM_COMMAND_WRITE   0x0000000C
#define COMMAND_REG_OFFSET 4
#define DEV_FIELD_SIZE (WS*2)//ogni device occupa 4=DEV_REG_SIZE word, ma i registri usati solo due

//struttura
struct list_head* select_io_queue(unsigned int deviceType, unsigned int deviceNumber);
//funzioni
void int_handler();
int get_priority_dev(memaddr* line);
void line_handler(int interruptLineNum);
void timer_handler();
void terminal_handler();
void device_handler(int dev_type);
void ack(int dev_type, int dev_numb, unsigned int status, memaddr *command_reg);
#endif
