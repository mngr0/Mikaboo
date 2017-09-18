#include <uARMconst.h>
#include "mikabooq.h"
#include "ssi.h"
#include "nucleus.h"
#include "exceptions.h"
#include "interrupts.h"
#include "const.h"
//controlla se il thread eliminato è un manager e in caso setta a null il campo adeguato
void free_managing(struct tcb_t* t_victim){
	if(t_victim->who_is_managing!=NULL){
		struct pcb_t* check=t_victim->who_is_managing;
		if (check->prg_mgr==t_victim){
			check->prg_mgr==NULL;
		}
		else if(check->sys_mgr==t_victim){
			check->sys_mgr==NULL;
		}
		else if(check->tlb_mgr==t_victim){
			check->tlb_mgr==NULL;
		}
	}
}
//controlla se ci sono thread nella wait queue che vogliono ricevere un messaggio da uno appena stato ucciso
void check_death(struct tcb_t* t_victim){
	struct tcb_t *t_temp=NULL;
	for_each_thread_in_q(t_temp,&wait_queue){
		if(t_temp->t_wait4sender==t_victim  ){
			//modifico la variabile  per gli error numb
			t_temp->err_numb=ERR_RECV_FROM_DEAD;
			//lo sveglio
			wake_me_up(t_temp);
			//non è importante il payload del messaggio
			*(unsigned int*)t_temp->t_s.a3= (unsigned int)NULL;
			//messaggio lo ha ricevuto da SSI
			t_temp->t_s.a1=(unsigned int)SSI;
			//vado solita istruzione dopo
			t_temp->t_s.pc+=WORD_SIZE;
			break;
		}
	}
}

//uccide thread
void __exterminate_thread(struct pcb_t * victim){
	//elimino ogni thread del processo
    while (!list_empty(&victim->p_threads)){
		//controllo che il thread eliminato non sia un manager
    	free_managing(proc_firstthread(victim));
    	//controllo che non ci siano figli che aspettano un messaggio da un defunto
        check_death(proc_firstthread(victim));
        //aggiorno il softblockcount
        if(thread_in_queue(&ready_queue,proc_firstthread(victim))){
          soft_block_count--;  
        }
        //tolgo il processo da qualsiasi lista si trova
        thread_outqueue(proc_firstthread(victim));
        thread_free(proc_firstthread(victim));
        //aggiorno il numero di thread
        thread_count--;  
    }
}
//uccide un processo e tutta la sua stirpe
void __exterminate_proc(struct pcb_t * victim){
	//uccido tutti i suoi thread
	__exterminate_thread(victim);
	struct pcb_t * temp;
	//per direttive iniziali, non possono esistere figli senza il padre, l'unico accettato è il root
	while (!list_empty(&victim->p_children)){
		temp=proc_firstchild(victim);
		//mi richiamo ricorsivamente sui figli
		__exterminate_proc(temp);
	}
	proc_delete(victim);
}
//chiama la vera funzione killer del processo
unsigned int ssi_terminate_process(struct tcb_t* sender){
	__exterminate_proc(sender->t_pcb);
	return FALSE;
}
//elimina un thread e in caso che non ci siano più thread di quel processo, uccide la sua stirpe
unsigned int ssi_terminate_thread(struct tcb_t* sender){
	struct pcb_t* parent=sender->t_pcb;
	//controllo che il thread eliminato non sia un manager
	free_managing(sender);
	//controllo che un thread non aspetti un messaggio da un morto
	check_death(sender);
	//lo tolgo da qualsiasi coda si trovi
	thread_outqueue(sender);
	thread_free(sender);
	//se non ci sono altri thread, allora elimino pure il processo che ha generato il thread
	if(list_empty(&parent->p_threads)){
		__exterminate_proc(parent);
	}
	//aggiorno numero thread
	thread_count--;
	return FALSE;
}
//gestione creazione processo e relativo thread
void ssi_create_process(state_t* state,struct tcb_t* sender, uintptr_t* reply){
	//creo processo e thread
	struct pcb_t* new_proc=proc_alloc(sender->t_pcb);
	struct tcb_t* new_thread= thread_alloc(new_proc);
	if(new_thread!=NULL){
		//salvo lo stato passato
		save_state(state,&new_thread->t_s);
		thread_count++;
		thread_enqueue(new_thread,&ready_queue);
	}
	*reply=(unsigned int)new_thread;
}
//gestione creazione thread
void ssi_create_thread(state_t * state,struct tcb_t* sender, uintptr_t* reply){
	//creo thread
	struct tcb_t* new= thread_alloc(sender->t_pcb);
	if(new!=NULL){
		//salvo lo stato passato
		save_state(state,&new->t_s);
		thread_count++;
		thread_enqueue(new,&ready_queue);
	}
	*reply=(unsigned int)new;
}
//assegno il prg mgr o killo il thread
unsigned int ssi_prg_managing(struct tcb_t* mgr,struct tcb_t* sender, uintptr_t* reply) {
	//se aveva già un manager
    if (sender->t_pcb->prg_mgr != NULL || mgr == NULL) {
       ssi_terminate_thread(sender);
        return FALSE;
    } else {
    	mgr->who_is_managing=sender->t_pcb;
        sender->t_pcb->prg_mgr = mgr;
        *reply=(unsigned int) NULL;
        return TRUE;
    }
}

//assegno il tlb mgr o killo il thread
unsigned int ssi_tlb_managing(struct tcb_t* mgr,struct tcb_t* sender, uintptr_t* reply) {
//se aveva già un manager
    if (sender->t_pcb->tlb_mgr != NULL || mgr == NULL) {
     ssi_terminate_thread(sender);
        return FALSE;
    } else {
    	mgr->who_is_managing=sender->t_pcb;
        sender->t_pcb->tlb_mgr = mgr;
        *reply=(unsigned int) NULL;
        return TRUE;
    }
}

//assegno il sys mgr o killo il thread
unsigned int ssi_sys_managing(struct tcb_t* mgr,struct tcb_t* sender,uintptr_t* reply) {
    //se aveva già un manager
    if (sender->t_pcb->sys_mgr != NULL || mgr == NULL){
     ssi_terminate_thread(sender);
        return FALSE;
    } else {
    	mgr->who_is_managing=sender->t_pcb;
        sender->t_pcb->sys_mgr = mgr;
        *reply=(unsigned int) NULL;
        return TRUE;
    }
}

void ssi_getcputime(struct tcb_t* sender, uintptr_t* reply){
	//la risposta è il valode del cputime del thread che lo ha chiesto
	*reply=sender->cpu_time;
}
cpu_t ssi_waitforclock(struct tcb_t* sender,uintptr_t* reply){
	*reply=(unsigned int)NULL;
	//azzero il tempo passato dall' inizio dell' attesa
	sender->elapsed_time = 0;
	//e lo inserisco nella wait_pseudo_clock_queue
	thread_outqueue(sender);
	//potrebbe essere ancora in ready queue, se il time slice è finito prima della receive
	if(sender->t_status==T_STATUS_READY)
		soft_block_count++;
	thread_enqueue(sender,&wait_pseudo_clock_queue);
	return FALSE;
}

//gestisco l input output
unsigned int ssi_do_io(uintptr_t * msg_ssi, struct tcb_t * sender){
	unsigned int dev_reg_com= *(msg_ssi+1);
	unsigned int dev_type=(dev_reg_com-DEV_REG_START)/(DEV_PER_INT*DEV_REG_SIZE)+DEV_IL_START;
	unsigned int dev_numb=((dev_reg_com-DEV_REG_START-COMMAND_REG_OFFSET)%(DEV_REG_SIZE*DEV_PER_INT))/DEV_FIELD_SIZE;
	//a questo punto dev_numb indica l' indice del campo COMMAND del device
	//equivale a (indice del device)*2 per tutti i device, 
	//tranne per i terminali in scrittura, dove vale (indice del device)*2+1
	if(dev_type==IL_TERMINAL){
		//se e' un terminale, controllo se e' in scrittura, in quel caso viene usata una diversa lista di coda
		if(dev_numb%2==1){
			dev_type+=1;
		}
	}
	//passo da (indice del device)*2 a (indice del device)
	dev_numb/=2;
	struct list_head* queue;
	queue=select_io_queue(dev_type,dev_numb);
	thread_outqueue(sender);
	if(sender->t_status==T_STATUS_READY)
		soft_block_count++;
	thread_enqueue(sender , queue );
	memaddr *base;
	switch (dev_type){
		case IL_DISK:
			break;
		case IL_TAPE:
			break;
		case IL_ETHERNET:
			break;
		case IL_TERMINAL:
			base = (memaddr *) ( dev_reg_com);
			*(base) = *(msg_ssi+2);
			break;
		case IL_TERMINAL+1:
			base = (memaddr *) (dev_reg_com);
			*(base) = *(msg_ssi+2);
			break;
	}



	return FALSE;
}


//ritorno il thread che ha fatto la richiesta
void ssi_get_mythreadid(struct tcb_t* sender, uintptr_t* reply ){
	*reply =(unsigned int)  sender;

}
//ritorno il processo padre del thread
void ssi_get_parentprocid(struct pcb_t* sender, uintptr_t* reply ){
	*reply =(unsigned int)  sender;

}
//ritorno il processo del thread
void ssi_get_processid(struct pcb_t* sender, uintptr_t* reply ){
	*reply =(unsigned int)  sender;
}
//ritorno l'error number
void ssi_get_erro(struct tcb_t* sender,uintptr_t* reply){
	*reply=sender->err_numb;
	sender->err_numb=NO_ERR;
}

//funzione principale dell SSI, controlla che il servizio sia un valore corretto e chiama la funzione corrispondente
unsigned int SSI_main_task(unsigned int * msg_ssi, struct tcb_t* sender ,uintptr_t* reply) {
	//controllo se non è una chiamata accettabile
   if (*msg_ssi < 0 || *msg_ssi > MAX_REQUEST_VALUE){
   	    ssi_terminate_thread(sender);
   }
	switch (*msg_ssi) {
		//in REPLY salviamo la risposta ottenuta
		case GET_ERRNO:
			ssi_get_erro(sender,reply);
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
			ssi_get_processid(((struct tcb_t*)*(msg_ssi+1))->t_pcb,reply);
			break;
		case GET_MYTHREADID:
			ssi_get_mythreadid(sender,reply);
			break;
		case GET_PARENTPROCID: 
			ssi_get_parentprocid(((struct pcb_t*)*(msg_ssi+1))->p_parent,reply);
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

