#ifndef SSI_H
#define SSI_H
//valore massimo della richiesta
#define MAX_REQUEST_VALUE 13
//funzioni


void exterminate_thread(struct pcb_t * victim);
void exterminate_proc(struct pcb_t * victim);
unsigned int ssi_terminate_process(struct tcb_t* sender);
unsigned int ssi_terminate_thread(struct tcb_t* sender);
void ssi_create_process(state_t* state,struct tcb_t* sender, uintptr_t* reply);
void ssi_create_thread(state_t * state,struct tcb_t* sender, uintptr_t* reply);
unsigned int ssi_prg_managing(struct tcb_t* mgr,struct tcb_t* sender, uintptr_t* reply);
unsigned int ssi_tlb_managing(struct tcb_t* mgr,struct tcb_t* sender, uintptr_t* reply);
unsigned int ssi_sys_managing(struct tcb_t* mgr,struct tcb_t* sender,uintptr_t* reply);
void ssi_getcputime();
unsigned int ssi_do_io(uintptr_t * msg_ssi, struct tcb_t * sender);
unsigned int ssi_get_mythreadid(struct tcb_t* sender, uintptr_t* reply );
unsigned int SSI_main_task(unsigned int * msg_ssi, struct tcb_t* sender ,uintptr_t* reply);
void ssi_entry();

#endif
