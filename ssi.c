

#include "mikabooq.h"
#include "ssi.h"
#include "nucleus.h"
/* Tramite questa funzione un thread può richiedere un servizio. Se la richiesta 
 * fatta dal Thread non esiste allora esso e tutta la sua progenie verranno uccisi.
 * Il thread che fa la richiesta deve obbligatoriamente restare in attesa di una 
 * risposta.
 * 
 * service: codice mnemonico che identifica il servizio
 * payload: contiene un'argomento, se richiesto, per il servizio
 * reply: punterà all'area dove verra messa la risposta nel caso sia richiesta
 */
void SSIRequest(unsigned int service, unsigned int payload, unsigned int *reply) { //qui modificare con le macro
    //struct msg_t msg_ssi;
    
    //msg_ssi.service = service;
    //msg_ssi.payload = payload;
    //msg_ssi.sender = currentThread;

    //MsgSend(SEND, SSI_MAGIC, (unsigned int) & msg_ssi);
    //MsgRecv(RECV, SSI_MAGIC, (unsigned int) & msg_ssi);

    //*reply = msg_ssi.reply;
}

unsigned int SSIdoRequest(struct msg_t * msg_ssi, unsigned int *reply) {

   // unsigned int service = msg_ssi->service;
   // unsigned int payload = msg_ssi->payload;
   // sender = msg_ssi->sender;

   // if (service < 1 || service > MAX_REQUEST_VALUE)/* Uccidire il thread chiamante*/
   //     terminate(sender);
        /*
    switch (service) {
            // service request values 
        case GETT_ERRNO:
            *reply = (unsigned int) createBrother((state_t *) payload);
            break;
        case CREATESON:
            *reply = (unsigned int) createSon((state_t *) payload);
            break;
        case TERMINATE:
            terminate(sender);
            break;
        case GETCPUTIME:
            *reply = (unsigned int) getCpuTime(sender);
            break;
        case SPECPRGMGR:    return specPrgMgr((tcb_t *)payload);
        case SPECTLBMGR:    return specTlbMgr((tcb_t *)payload);
        case SPECSYSMGR:    return specSysMgr((tcb_t *)payload);
        case WAITFORCLOCK:  return waitForClock(sender);
        case WAITFORIO:     return waitForIO(msg_ssi);
        case WAKE_UP_PSEUDO_CLOCK:   return wakeUpPseudoClock();
        case WAKE_UP_FROM_IO:   return wakeUpFromIO(msg_ssi);

	}
	*/
    return TRUE;
}

void ssi_entry() {
    unsigned int toBeSent;
    unsigned int msg;
    struct tcb_t* sender;
    for (;;) {

        
        sender = msgrecv(NULL, &msg);
       // toBeSent = SSIdoRequest(&msg, &(msg.reply));

        if (toBeSent)
            msgsend((memaddr) sender, &msg);
    }
}

