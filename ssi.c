

#include "mikabooq.h"
#include "ssi.h"
#include "nucleus.h"
#include "scheduler.h"

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

void ssi_do_io(struct dev_acc_ctrl device , uintptr_t command,uintptr_t data1,uintptr_t data2){
	//controlla e scrive se necessario
	//controllare -> device.state
	//scrivere 
}

unsigned int SSIdoRequest(unsigned int * msg_ssi, struct tcb_t* sender ,uintptr_t reply) {
	unsigned int service;
	service=*msg_ssi;
	char t= 'd';
	char *s=&t;
	memaddr * base;
   // unsigned int payload = msg_ssi->payload;
   // sender = msg_ssi->sender;

   // if (service < 1 || service > MAX_REQUEST_VALUE)/* Uccidire il thread chiamante*/
   //     terminate(sender);

	switch (service) {
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
		break;
		case SETTLBMGR:
		break;
		case SETSYSMGR:
		break;
		case GET_CPUTIME :
		break;
		case WAIT_FOR_CLOCK:
		break;
		case DO_IO:
		//incoda nella coda giusta e chiama la funzione che controlla e stampa
		base = (memaddr *) (TERM0ADDR);
		*(base) = 2 | (((memaddr) t) << 8);


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

void ssi_entry() {
	unsigned int toBeSent;
	uintptr_t msg;
	uintptr_t reply;
	struct tcb_t* sender;
	for (;;) {


		sender = msgrecv(NULL, msg);
		toBeSent = SSIdoRequest(&msg, sender,reply);

		if (toBeSent)
			msgsend((memaddr) sender, reply);
	}
}

