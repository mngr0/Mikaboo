

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

unsigned int specPrgMgr(struct tcb_t* mgr,struct tcb_t* sender) {
    if (sender->t_pcb->prgMgr != NULL || mgr == NULL) {
  //      terminate(sender);
        return FALSE;
    } else {
        sender->t_pcb->prgMgr = mgr;
        return TRUE;
    }
}

unsigned int specTlbMgr(struct tcb_t* mgr,struct tcb_t* sender) {
    if (sender->t_pcb->tlbMgr != NULL || mgr == NULL) {
    //    terminate(sender);
        return FALSE;
    } else {
        sender->t_pcb->tlbMgr = mgr;
        return TRUE;
    }
}
unsigned int specSysMgr(struct tcb_t* mgr,struct tcb_t* sender) {
    if (sender->t_pcb->sysMgr != NULL || mgr == NULL){
     //   terminate(sender);
        return FALSE;
    } else {
        sender->t_pcb->sysMgr = mgr;
        return TRUE;
    }
}
void DPHERE(){}
void ssi_do_io(uintptr_t * msg_ssi, struct tcb_t * sender){
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
	DPHERE();
}

unsigned int SSIdoRequest(unsigned int * msg_ssi, struct tcb_t* sender ,uintptr_t reply) {
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
		   //*reply = (unsigned int) createSon((state_t *) payload);
		break;
		case CREATE_THREAD:
		//terminate(sender);
		break;
		case TERMINATE_PROCESS:
		break;
		case TERMINATE_THREAD :
		break;
		case SETPGMMGR:
			specPrgMgr((struct tcb_t*)*(msg_ssi+1),sender);
		break;
		case SETTLBMGR:
			specTlbMgr((struct tcb_t*)*(msg_ssi+1),sender);
		break;
		case SETSYSMGR:
			specSysMgr((struct tcb_t*)*(msg_ssi+1),sender);
		break;	
		case GET_CPUTIME :
		break;
		case WAIT_FOR_CLOCK:
		break;
		case DO_IO:
			ssi_do_io(msg_ssi,sender);
			break;
		case GET_PROCESSID :
		break;
		case GET_MYTHREADID :
		break;
		case GET_PARENTPROCID : 
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
		ACHERE();
		toBeSent = SSIdoRequest((uintptr_t*)msg, sender,reply);

		if (toBeSent)
			msgsend((memaddr) sender, reply);
	}
}

