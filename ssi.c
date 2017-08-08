

#include "mikabooq.h"
#include "ssi.h"
#include "nucleus.h"
#include "scheduler.h"
#include "exceptions.h"
unsigned int* service;
struct dev_acc_ctrl* q;

//void SSIRequest(unsigned int service, unsigned int payload, unsigned int *reply) { //qui modificare con le macro

//}

void exterminate_thread(struct pcb_t * victim){
    while (!list_empty(&victim->p_threads)){
        if(out_thread(&readyQueue,proc_firstthread(victim))==NULL){
          softBlockCount--;  
        }
        else{
            thread_outqueue(proc_firstthread(victim));    
        }
        
        thread_free(proc_firstthread(victim));
        threadCount--;
    }
}

void exterminate_proc(struct pcb_t * victim){
	exterminate_thread(victim);
	struct pcb_t * temp;
	while (!list_empty(&victim->p_children)){
		temp=proc_firstchild(victim->p_parent);
		exterminate_proc(temp);
	}
	proc_delete(victim);
}

unsigned int ssi_terminate_process(struct tcb_t* sender){
	exterminate_proc(sender->t_pcb);
	return FALSE;
}

unsigned int ssi_terminate_thread(struct tcb_t* sender){
	struct pcb_t* parent=sender->t_pcb;
	thread_outqueue(sender);
	thread_free(sender);
	if(list_empty(&parent->p_threads)){
		exterminate_proc(parent);
	}
	return FALSE;
}

void ssi_create_process(state_t* state,struct tcb_t* sender, uintptr_t* reply){
	struct pcb_t* new_proc=proc_alloc(sender->t_pcb);
	struct tcb_t* new_thread= thread_alloc(new_proc);
	if(new_thread!=NULL){
		saveStateIn(state,&new_thread->t_s);
	}
	*reply=(unsigned int)new_thread;
}

void ssi_create_thread(state_t * state,struct tcb_t* sender, uintptr_t* reply){
	struct tcb_t* new= thread_alloc(sender->t_pcb);
	if(new!=NULL){
		saveStateIn(state,&new->t_s);
	}
	*reply=(unsigned int)new;
}

unsigned int specPrgMgr(struct tcb_t* mgr,struct tcb_t* sender, uintptr_t* reply) {

    if (sender->t_pcb->prgMgr != NULL || mgr == NULL) {
  //      terminate(sender);
        return FALSE;
    } else {
    *reply=NULL;

        sender->t_pcb->prgMgr = mgr;
        *reply=(unsigned int) NULL;
        return TRUE;
    }
}


unsigned int specTlbMgr(struct tcb_t* mgr,struct tcb_t* sender, uintptr_t* reply) {

    if (sender->t_pcb->tlbMgr != NULL || mgr == NULL) {
    //    terminate(sender);
        return FALSE;
    } else {
    *reply=NULL;

        sender->t_pcb->tlbMgr = mgr;
        *reply=(unsigned int) NULL;
        return TRUE;
    }
}


unsigned int specSysMgr(struct tcb_t* mgr,struct tcb_t* sender,uintptr_t* reply) {
    if (sender->t_pcb->sysMgr != NULL || mgr == NULL){
     //   terminate(sender);
        return FALSE;
    } else {
        sender->t_pcb->sysMgr = mgr;

        *reply=(unsigned int) NULL;
        return TRUE;
    }
}

unsigned int ssi_getcputime(){}

unsigned int ssi_do_io(uintptr_t * msg_ssi, struct tcb_t * sender){
	//controlla e scrive se necessario
	//controllare -> device.state
	//scrivere 
	q=select_io_queue_from_status_addr( *(msg_ssi+1));
	thread_outqueue(sender);
	thread_enqueue(sender , &q->acc );
	//thread_enqueue(currentThread , &waitingQueue);
	//stampare
	memaddr *base = (memaddr *) ( *(msg_ssi+1));
	*(base) = *(msg_ssi+2);
	return FALSE;
}
void ssi_create_thread(state_t* state,struct tcb_t* sender,uintptr_t * reply){
    struct tcb_t* new=thread_alloc(sender->t_pcb);
    if(new!=NULL)
        saveStateIn(state,&(new->t_s));
    *reply=(unsigned int)new;
}
void ssi_create_process(state_t* state,struct tcb_t* sender,uintptr_t * reply){
    struct pcb_t* new_proc=proc_alloc(sender->t_pcb);
    struct tcb_t* new_thread=thread_alloc(new_proc);
    if(new_thread!=NULL)
        saveStateIn(state,&(new_thread->t_s));

    *reply=(unsigned int)new_thread;
}
unsigned int ssi_terminate_thread(struct tcb_t* sender){
    struct pcb_t* parent=sender->t_pcb;
    thread_outqueue(sender);
    thread_free(sender);
    if(proc_firstthread(parent)==NULL){
        //ammazzo tutti i figli e i suoi thread
        exterminate_proc(parent);
        //suicidio eroico
        proc_delete(parent);
    }
    threadCount--;
    return FALSE;
}
unsigned int ssi_terminate_process(struct tcb_t * sender){
    struct pcb_t* parent=sender->t_pcb;
    exterminate_proc(parent);
    return FALSE;


unsigned int ssi_get_mythreadid(struct tcb_t* sender, uintptr_t* reply ){
	*reply =(unsigned int)  sender;
	return TRUE;

}

unsigned int SSIdoRequest(unsigned int * msg_ssi, struct tcb_t* sender ,uintptr_t* reply) {
	service=msg_ssi;
   // unsigned int payload = msg_ssi->payload;
   // sender = msg_ssi->sender;

   // if (service < 1 || service > MAX_REQUEST_VALUE)/* Uccidire il thread chiamante*/
   //     terminate(sender);

	switch (*service) {
        // service request values 
		case GET_ERRNO:

		//*reply = (unsigned int) createBrother((state_t *) payload);
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
			return specPrgMgr((struct tcb_t*)*(msg_ssi+1),sender,reply);
			break;
		case SETTLBMGR:
			return specTlbMgr((struct tcb_t*)*(msg_ssi+1),sender,reply);
			break;
		case SETSYSMGR:
			return specSysMgr((struct tcb_t*)*(msg_ssi+1),sender,reply);
			break;	

		case GET_CPUTIME:
			return TRUE;

		case WAIT_FOR_CLOCK:
			break;

		case DO_IO:

			return ssi_do_io(msg_ssi,sender);
			break;
		case GET_PROCESSID :
			*reply =(unsigned int) sender->t_pcb;
			break;
		case GET_MYTHREADID:
			*reply =(unsigned int)  sender;
			break;
		case GET_PARENTPROCID: 
			*reply =(unsigned int)  sender->t_pcb->p_parent;
			break;

	}
	return TRUE;
}

void ssi_entry() {
	struct tcb_t* sender;
	unsigned int toBeSent;
	uintptr_t msg;
	uintptr_t reply;
	for (;;) {
		sender = msgrecv(NULL,&msg);

		toBeSent = SSIdoRequest((uintptr_t*)msg, sender,&reply);
		if (toBeSent)
			msgsend((memaddr) sender, &reply);

	}
}

