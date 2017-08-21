#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H
//funzioni
void tlb_handler();
void pgm_handler();
void sys_bp_handler();

void sys_send_msg(struct tcb_t* sender,struct tcb_t* receiver,unsigned int msg);


void put_thread_sleep(struct tcb_t* t);
void wake_me_up(struct tcb_t* sleeper);
void check_thread_alive(struct tcb_t * t,int cause);

void save_state(state_t *from, state_t *to);
extern state_t *tlb_old;
extern state_t *sysbp_old;
extern state_t *pgmtrap_old;
#endif
