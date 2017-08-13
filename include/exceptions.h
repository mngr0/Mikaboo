#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#define ERR_UNKNOWN 0
#define ERR_SEND_TO_DEAD 1
#define ERR_RECV_FROM_DEAD 2
//funzioni
unsigned int err_numb;
void tlb_handler();
void pgm_handler();
void sys_send_msg();
void sys_recv_msg();
void sys_bp_handler();
void save_state(state_t *from, state_t *to);

#endif
