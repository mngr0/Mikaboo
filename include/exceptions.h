#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H
//funzioni
void tlb_handler();
void pgm_handler();
void sys_send_msg();
void sys_recv_msg();
void sys_bp_handler();
void save_state(state_t *from, state_t *to);

#endif
