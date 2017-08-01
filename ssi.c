/* Tramite questa funzione un thread può richiedere un servizio. Se la richiesta 
 * fatta dal Thread non esiste allora esso e tutta la sua progenie verranno uccisi.
 * Il thread che fa la richiesta deve obbligatoriamente restare in attesa di una 
 * risposta.
 * 
 * service: codice mnemonico che identifica il servizio
 * payload: contiene un'argomento, se richiesto, per il servizio
 * reply: punterà all'area dove verra messa la risposta nel caso sia richiesta
 */
void SSIRequest(U32 service, U32 payload, U32 *reply) { //qui modificare con le macro
    ssimsg_t msg_ssi;
    
    msg_ssi.service = service;
    msg_ssi.payload = payload;
    msg_ssi.sender = currentThread;

    MsgSend(SEND, SSI_MAGIC, (U32) & msg_ssi);
    MsgRecv(RECV, SSI_MAGIC, (U32) & msg_ssi);

    *reply = msg_ssi.reply;
}

unsigned int SSIdoRequest(ssimsg_t * msg_ssi, U32 *reply) {

    unsigned int service = msg_ssi->service;
    unsigned int payload = msg_ssi->payload;
    sender = msg_ssi->sender;

    if (service < 1 || service > MAX_REQUEST_VALUE)/* Uccidire il thread chiamante*/
        terminate(sender);

    switch (service) {
            /* service request values */
        case GETT_ERRNO:
            *reply = (U32) createBrother((state_t *) payload);
            break;
        case CREATESON:
            *reply = (U32) createSon((state_t *) payload);
            break;
        case TERMINATE:
            terminate(sender);
            break;
        case GETCPUTIME:
            *reply = (U32) getCpuTime(sender);
            break;
        case SPECPRGMGR:    return specPrgMgr((tcb_t *)payload);
        case SPECTLBMGR:    return specTlbMgr((tcb_t *)payload);
        case SPECSYSMGR:    return specSysMgr((tcb_t *)payload);
        case WAITFORCLOCK:  return waitForClock(sender);
        case WAITFORIO:     return waitForIO(msg_ssi);
        case WAKE_UP_PSEUDO_CLOCK:   return wakeUpPseudoClock();
        case WAKE_UP_FROM_IO:   return wakeUpFromIO(msg_ssi);

    }
    return TRUE;
}

void ssi_entry() {
    U32 toBeSent;
    ssimsg_t msg;
    for (;;) {

        sender = MsgRecv(RECV, ANYMESSAGE, &msg);
        toBeSent = SSIdoRequest(&msg, &(msg.reply));

        if (toBeSent)
            MsgSend(SEND, (U32) sender, &msg);
    }
}

