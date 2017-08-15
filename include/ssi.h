#ifndef SSI_H
#define SSI_H

//funzioni
void check_death(struct tcb_t* t_victim);
void exterminate_thread(struct pcb_t * victim);
void exterminate_proc(struct pcb_t * victim);

unsigned int ssi_terminate_process(struct tcb_t* sender);
unsigned int ssi_terminate_thread(struct tcb_t* sender);

unsigned int ssi_prg_managing(struct tcb_t* mgr,struct tcb_t* sender, uintptr_t* reply);
unsigned int ssi_tlb_managing(struct tcb_t* mgr,struct tcb_t* sender, uintptr_t* reply);
unsigned int ssi_sys_managing(struct tcb_t* mgr,struct tcb_t* sender,uintptr_t* reply);

unsigned int ssi_waitforclock(struct tcb_t* sender,uintptr_t* reply);
unsigned int ssi_do_io(uintptr_t * msg_ssi, struct tcb_t * sender);

void ssi_create_process(state_t* state,struct tcb_t* sender, uintptr_t* reply);
void ssi_create_thread(state_t * state,struct tcb_t* sender, uintptr_t* reply);
void ssi_getcputime();

void ssi_get_mythreadid(struct tcb_t* sender, uintptr_t* reply );
void ssi_get_parentprocid(struct pcb_t* sender, uintptr_t* reply );
void ssi_get_processid(struct pcb_t* sender, uintptr_t* reply );
void ssi_get_erro(struct tcb_t* sender,uintptr_t* reply);

unsigned int SSI_main_task(unsigned int * msg_ssi, struct tcb_t* sender ,uintptr_t* reply);
void ssi_entry();

#endif
