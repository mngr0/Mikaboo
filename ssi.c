

#include "mikabooq.h"
#include "ssi.h"
#include "nucleus.h"
#include "scheduler.h"
unsigned int* service;
struct dev_acc_ctrl* q;

/* Tramite questa funzione un thread può richiedere un servizio. Se la richiesta 
 * fatta dal Thread non esiste allora esso e tutta la sua progenie verranno uccisi.
 * Il thread che fa la richiesta deve obbligatoriamente restare in attesa di una 
 * risposta.
 * 
 * service: codice mnemonico che identifica il servizio
 * payload: contiene un'argomento, se richiesto, per il servizio
 * reply: punterà all'area dove verra messa la risposta nel caso sia richiesta
 */
//void SSIRequest(unsigned int service, unsigned int payload, unsigned int *reply) { //qui modificare con le macro
    //struct msg_t msg_ssi;

    //msg_ssi.service = service;
    //msg_ssi.payload = payload;
    //msg_ssi.sender = currentThread;

    //MsgSend(SEND, SSI_MAGIC, (unsigned int) & msg_ssi);
    //MsgRecv(RECV, SSI_MAGIC, (unsigned int) & msg_ssi);

    // *reply = msg_ssi.reply;
//}
void exterminate_thread(struct pcb_t * victim){
    while (!list_empty(&victim->p_threads)){
        if(out_thread(&readyQueue,victim)==NULL){
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
unsigned int specPrgMgr(struct tcb_t* mgr,struct tcb_t* sender,uintptr_t* reply) {
    if (sender->t_pcb->prgMgr != NULL || mgr == NULL) {
  //      terminate(sender);
        return FALSE;
    } else {
    *reply=NULL;

        sender->t_pcb->prgMgr = mgr;
        return TRUE;
    }
}

unsigned int specTlbMgr(struct tcb_t* mgr,struct tcb_t* sender,uintptr_t* reply) {
    if (sender->t_pcb->tlbMgr != NULL || mgr == NULL) {
    //    terminate(sender);
        return FALSE;
    } else {
    *reply=NULL;

        sender->t_pcb->tlbMgr = mgr;
        return TRUE;
    }
}
unsigned int specSysMgr(struct tcb_t* mgr,struct tcb_t* sender,uintptr_t* reply) {
    if (sender->t_pcb->sysMgr != NULL || mgr == NULL){
     //   terminate(sender);
        return FALSE;
    } else {
        sender->t_pcb->sysMgr = mgr;
    *reply=NULL;

        return TRUE;
    }
}
void DPHERE(){}
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
               ssi_create_process((state_t *)*(msg_ssi+1),sender,reply);

        break;
        case CREATE_THREAD:
           ssi_create_thread((state_t *)*(msg_ssi+1),sender,reply);

        
        break;
		case TERMINATE_PROCESS:
		break;
		case TERMINATE_THREAD :
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
		case GET_CPUTIME :
		break;
		case WAIT_FOR_CLOCK:
		break;
		case DO_IO:
			
            return ssi_do_io(msg_ssi,sender);
			break;
        case GET_PROCESSID :
            *reply=(unsigned int) sender->t_pcb;
        break;
        case GET_MYTHREADID :
            *reply=(unsigned int) sender->t_pcb;
        break;
        case GET_PARENTPROCID : 
            *reply=(unsigned int)sender->t_pcb->p_parent;
        break;
	}
	return TRUE;
}
void ACHERE(){}


void ssi_entry() {
	struct tcb_t* sender;

	unsigned int toBeSent;

	uintptr_t msg;
	uintptr_t reply;
	for (;;) {

		
		sender = msgrecv(NULL,&msg);
		
		toBeSent = SSIdoRequest((uintptr_t*)msg, sender,&reply);

		if (toBeSent)
			msgsend((memaddr) sender,& reply);
	}
}

