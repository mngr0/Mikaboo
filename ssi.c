#include "mikabooq.h"
#include "ssi.h"
#include "nucleus.h"
#include "scheduler.h"
#include "exceptions.h"

//void SSIRequest(unsigned int service, unsigned int payload, unsigned int *reply) { //qui modificare con le macro
//}
void CA(){}


void check_death(struct tcb_t* t_victim){
	struct tcb_t *t_temp=NULL;
	for_each_thread_in_q(t_temp,&wait_queue){
		if(t_temp->t_wait4sender==t_victim  ){
			CA();
			wake_me_up(t_victim,t_temp,NULL);
		}
	}
}


//uccide thread
void exterminate_thread(struct pcb_t * victim){
    while (!list_empty(&victim->p_threads)){
        if(out_thread(&ready_queue,proc_firstthread(victim))==NULL){
          soft_block_count--;  
        }
        else{
            thread_outqueue(proc_firstthread(victim));
        }
        check_death(proc_firstthread(victim));
        thread_count--;
        thread_free(proc_firstthread(victim));
        
    }
}
//uccide un processo e tutta la sua stirpe
void exterminate_proc(struct pcb_t * victim){
	exterminate_thread(victim);
	struct pcb_t * temp;
	while (!list_empty(&victim->p_children)){
		temp=proc_firstchild(victim->p_parent);
		exterminate_proc(temp);
	}
	proc_delete(victim);
}
//chiama la vera funzione killer del processo
unsigned int ssi_terminate_process(struct tcb_t* sender){
	exterminate_proc(sender->t_pcb);
	return FALSE;
}
//elimina un thread e in caso che non ci siano più thread di quel processo, uccide la sua stirpe
unsigned int ssi_terminate_thread(struct tcb_t* sender){
	struct pcb_t* parent=sender->t_pcb;
	thread_outqueue(sender);
	check_death(sender);
	thread_free(sender);
	if(list_empty(&parent->p_threads)){
		exterminate_proc(parent);
	}
	thread_count--;
	return FALSE;
}
//gestione creazione processo e relativo thread
void ssi_create_process(state_t* state,struct tcb_t* sender, uintptr_t* reply){
	struct pcb_t* new_proc=proc_alloc(sender->t_pcb);
	struct tcb_t* new_thread= thread_alloc(new_proc);
	if(new_thread!=NULL){
		save_state(state,&new_thread->t_s);
		thread_count++;
		thread_enqueue(new_thread,&ready_queue);
	}
	*reply=(unsigned int)new_thread;
}
//gestione creazione thread
void ssi_create_thread(state_t * state,struct tcb_t* sender, uintptr_t* reply){
	struct tcb_t* new= thread_alloc(sender->t_pcb);
	//struct tcb_t * Ashow=new; //NON VEDO L'UTILITA DI QUESTO, PER ORA E' COMMENTATO
	if(new!=NULL){
		save_state(state,&new->t_s);
		thread_count++;
		thread_enqueue(new,&ready_queue);
	}
	*reply=(unsigned int)new;
}
//assegno il prg mgr o killo il thread
unsigned int ssi_prg_managing(struct tcb_t* mgr,struct tcb_t* sender, uintptr_t* reply) {

    if (sender->t_pcb->prgMgr != NULL || mgr == NULL) {
       ssi_terminate_thread(sender);
        return FALSE;
    } else {

        sender->t_pcb->prgMgr = mgr;
        *reply=(unsigned int) NULL;
        return TRUE;
    }
}

//assegno il tlb mgr o killo il thread
unsigned int ssi_tlb_managing(struct tcb_t* mgr,struct tcb_t* sender, uintptr_t* reply) {

    if (sender->t_pcb->tlbMgr != NULL || mgr == NULL) {
     ssi_terminate_thread(sender);
        return FALSE;
    } else {

        sender->t_pcb->tlbMgr = mgr;
        *reply=(unsigned int) NULL;
        return TRUE;
    }
}

//assegno il sys mgr o killo il thread
unsigned int ssi_sys_managing(struct tcb_t* mgr,struct tcb_t* sender,uintptr_t* reply) {
    if (sender->t_pcb->sysMgr != NULL || mgr == NULL){
     ssi_terminate_thread(sender);
        return FALSE;
    } else {
        sender->t_pcb->sysMgr = mgr;

        *reply=(unsigned int) NULL;
        return TRUE;
    }
}

void ssi_getcputime(struct tcb_t* sender, uintptr_t* reply){
	*reply=sender->cpu_time;
}
unsigned int ssi_waitforclock(struct tcb_t* sender,uintptr_t* reply){
	*reply=(unsigned int)NULL;
	waiting_TOD=getTODLO();
	thread_outqueue(sender);
	thread_enqueue(sender,&wait_pseudo_clock_queue);
	return FALSE;
}

//gestisco l input output
unsigned int ssi_do_io(uintptr_t * msg_ssi, struct tcb_t * sender){
	unsigned int dev_reg_com= *(msg_ssi+1);
	unsigned int dev_type=IL_TERMINAL;//TODO
	unsigned int dev_numb=0;
	struct list_head* queue;
	queue=select_io_queue(dev_type,dev_numb);
	//q=&device_list[(dev_type-3)*DEV_PER_INT+dev_numb];
	thread_outqueue(sender);
	thread_enqueue(sender , queue );
	//thread_enqueue(current_thread , &wait_queue);
	//stampare
	memaddr *base = (memaddr *) ( *(msg_ssi+1));
	*(base) = *(msg_ssi+2);
	return FALSE;
}


//ritorno il thread che ha fatto la richiesta
unsigned int ssi_get_mythreadid(struct tcb_t* sender, uintptr_t* reply ){
	*reply =(unsigned int)  sender;
	return TRUE;

}


//funzione principale dell SSI, controlla che il servizio sia un valore corretto e chiama la funzione corrispondente
unsigned int SSI_main_task(unsigned int * msg_ssi, struct tcb_t* sender ,uintptr_t* reply) {
	unsigned int* service=msg_ssi;


   if (*service < 0 || *service > MAX_REQUEST_VALUE)
   		//uccido chiamante
        ssi_terminate_thread(sender);

	switch (*service) {
        // valori della richiesta 
		case GET_ERRNO:
			break;
		case CREATE_PROCESS:
			ssi_create_process((state_t*)*(msg_ssi+1),sender,reply);
			break;
		case CREATE_THREAD:
			ssi_create_thread(((state_t*)*(msg_ssi+1)),sender,reply);
			break;
		case TERMINATE_PROCESS:
			return ssi_terminate_process(sender);
			break;
		case TERMINATE_THREAD:
			return ssi_terminate_thread(sender);
			break;
		case SETPGMMGR:
			return ssi_prg_managing((struct tcb_t*)*(msg_ssi+1),sender,reply);
			break;
		case SETTLBMGR:
			return ssi_tlb_managing((struct tcb_t*)*(msg_ssi+1),sender,reply);
			break;
		case SETSYSMGR:
			return ssi_sys_managing((struct tcb_t*)*(msg_ssi+1),sender,reply);
			break;	

		case GET_CPUTIME:
			ssi_getcputime(sender,reply);
			break;

		case WAIT_FOR_CLOCK:
			return ssi_waitforclock(sender,reply);
			break;

		case DO_IO:

			return ssi_do_io(msg_ssi,sender);
			break;
		case GET_PROCESSID :
			*reply =(unsigned int)( ((struct tcb_t*)*(msg_ssi+1))->t_pcb);
			break;
		case GET_MYTHREADID:
			*reply =(unsigned int)  sender;
			break;
		case GET_PARENTPROCID: 
			*reply =(unsigned int) ( ((struct pcb_t*)*(msg_ssi+1))->p_parent );
			break;

	}
	return TRUE;
}
//funzione che viene chiamata all'attivazione dell'ssi, decide se l ssi dovrà pure rispondere al thread che ha inviato il messaggio
void ssi_entry() {
	uintptr_t reply; //risposta
	struct tcb_t* sender;	//mittente
	unsigned int has_to_sent;	//è da mandare messaggio o meno
	uintptr_t msg;	//messaggio 

	for (;;) {
		sender = msgrecv(NULL,&msg);
		has_to_sent = SSI_main_task((uintptr_t*)msg, sender,&reply);
		if (has_to_sent) //ssi deve rispondere
			msgsend((memaddr) sender, reply);

	}
}

