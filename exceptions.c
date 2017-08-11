#include <libuarm.h>
#include <uARMconst.h>
#include <uARMtypes.h>
#include <arch.h>
#include "const.h"
#include "mikabooq.h"
#include "scheduler.h"
#include "nucleus.h"
#include "exceptions.h"

state_t *tlb_old   = (state_t*) TLB_OLDAREA;
state_t *pgmtrap_old = (state_t*) PGMTRAP_OLDAREA;
state_t *sysbp_old   = (state_t*) SYSBK_OLDAREA;

void save_state(state_t *from, state_t *to){
	to->a1                  = from->a1;
	to->a2                  = from->a2;
	to->a3                  = from->a3;
	to->a4                  = from->a4;
	to->v1                  = from->v1;
	to->v2                  = from->v2;
	to->v3                  = from->v3;
	to->v4                  = from->v4;
	to->v5                  = from->v5;
	to->v6                  = from->v6;
	to->sl                  = from->sl;
	to->fp                  = from->fp;
	to->ip                  = from->ip;
	to->sp                  = from->sp;
	to->lr                  = from->lr;
	to->pc                  = from->pc;
	to->cpsr                = from->cpsr;
	to->CP15_Control        = from->CP15_Control;
	to->CP15_EntryHi        = from->CP15_EntryHi;
	to->CP15_Cause          = from->CP15_Cause;
	to->TOD_Hi              = from->TOD_Hi;
	to->TOD_Low             = from->TOD_Low;
}

void tlb_handler(){
    // Se un processo è eseguito dal processore salvo lo stato nella tlb_oldarea 
	/*
	if(currentProcess != NULL){
		save_state(tlb_old, &currentProcess->p_s);
	}

	useExStVec(SPECTLB);
	*/
}

void pgm_handler(){
    //se un processo è eseguito dal processore salvo lo stato nella pgmtrap_oldarea*/
	/* if(currentProcess != NULL){
		save_state(pgmtrap_old, &currentProcess->sp);
	}
	useExStVec(SPECPGMT);
	*/
}

void sys_send_msg(struct tcb_t* sender,struct tcb_t* receiver,unsigned int msg){
	int msg_res;
	//se il destinatario è in attesa proprio di questo messaggio
	if( (receiver->t_status==T_STATUS_W4MSG) && ( (receiver->t_wait4sender==sender) || (receiver->t_wait4sender==NULL) ) ){
		//lo sveglio
		thread_outqueue(receiver);
		thread_enqueue(receiver,&ready_queue);
		//setto i vari campi
		* (unsigned int *)receiver->t_s.a3=msg;
		receiver->t_s.a1=(unsigned int)sender;
		receiver->t_status=T_STATUS_READY;
		//faccio in modo che non rientri nel codice della sys call
		receiver->t_s.pc += WORD_SIZE;
		//se la funzione sys_send_msg è stata chiamata dall' interrupt handler
		//mando un messaggio da parte dell' ssi, ma non vado a modificare i suoi registri
		if (sender!=SSI)
			sender->t_s.a1=0;
		soft_block_count--;
	}
	//altrimenti lo incodo normalmente
	else{
		msg_res=msgq_add(sender,receiver,msg);
		//coda piena, il messaggio non è stato inviato, la msgsend ritornerà -1 CHECK
		if(msg_res==-1){
			//se la funzione sys_send_msg è stata chiamata dall' interrupt handler
			//mando un messaggio da parte dell' ssi, ma non vado a modificare i suoi registri
			if (sender!=SSI)
				sender->t_s.a1=-1;
		}
	}

}

void sys_recv_msg(){}



//gestione system calle breaking point
void sys_bp_handler(){
	//salvo lo stato del thread corrente
	save_state(sysbp_old, &current_thread->t_s);
	//controllo perchè è stato chiamato il sys_bp_handler
	unsigned int cause = CAUSE_EXCCODE_GET(sysbp_old->CP15_Cause);

	unsigned int a0 = sysbp_old->a1;
	struct tcb_t * a1 =(struct tcb_t *) sysbp_old->a2;
	uintptr_t a2 = sysbp_old->a3;
	//salvo messaggio
	int msg_res;
	// Se l'eccezione è di tipo System call 
	if(cause==EXC_SYSCALL){
    	// Se il processo è in kernel mode gestisce adeguatamente 
		if( (current_thread->t_s.cpsr & STATUS_SYS_MODE) == STATUS_SYS_MODE){
			switch(a0){
				//errore
				case 0:
					PANIC();
				//devo inviare un messaggio
				case SYS_SEND:
	                //a0 contiene la costante 1 (messaggio inviato)
	                //a1 contiene l'indirizzo del thread destinatario
        			//a2 contiene il puntatore al messaggio

					//faccio la send
					sys_send_msg(current_thread,a1,a2);
					// Evito che rientri nel codice della syscall
					current_thread->t_s.pc += WORD_SIZE;
					LDST(&current_thread->t_s);
				break;
				//devo ricevere un messaggio
				case SYS_RECV:
					//a0 contiene costante 2
					//a1 contiene l'indirizzo del mittente(null==tutti)
					//a2 contiene puntatore al campo dove regitrare il messaggio(NULL== non registrare)
					msg_res=msgq_get(&a1,current_thread,(uintptr_t *)a2);
					//non c'è ancora il messaggio
					if (msg_res==-1){
						//non è più il thread corrente
						thread_outqueue(current_thread);
						//perchè va in attessa
						thread_enqueue(current_thread,&wait_queue);
						//sistemo i vari campi
						current_thread->t_wait4sender=a1;
						current_thread->t_s.a3=a2;
						current_thread->t_status=T_STATUS_W4MSG;
						current_thread->cpu_time+=getTODLO()-process_TOD;
						current_thread=NULL;
						soft_block_count++;
					}
					//caso corretto
					else{
						//metto in t_s.a3 il puntatore alla struct messaggio
						current_thread->t_s.a3=a2;
						//la msgrecv ritornerà un puntatore al mittente
						current_thread->t_s.a1=(unsigned int)a1;
						// Evito che rientri nel codice della syscall
						
						current_thread->t_s.pc += WORD_SIZE;
						//carico il thread
						LDST(&current_thread->t_s);
					}
					break;
				default:
					//check TUTTI I PUNTATORI
				    //se hanno un sysmgr adeguato
				    if(current_thread->t_pcb->sysMgr != NULL) {
						msgq_add(current_thread,current_thread->t_pcb->sysMgr,(uintptr_t)&(current_thread->t_s.CP15_Cause));
						//sveglio il manager
						if (out_thread(&wait_queue,current_thread->t_pcb->sysMgr)!=NULL) {
					   				 thread_enqueue(current_thread->t_pcb->sysMgr,&ready_queue);
                       				 soft_block_count--;
                   		}
                   		//blocco il processo corrente
						thread_outqueue(current_thread);
						thread_enqueue(current_thread,&wait_queue);
               		  }
               		//se  hanno un program pgr adeguato
				    else if(current_thread->t_pcb->prgMgr != NULL) {
						msgq_add(current_thread,current_thread->t_pcb->prgMgr,(unsigned int)&(current_thread->t_s.CP15_Cause));
						//sveglio il manager
						if (out_thread(&wait_queue,current_thread->t_pcb->prgMgr)!=NULL) {
							thread_enqueue(current_thread->t_pcb->prgMgr,&ready_queue);
                       		soft_block_count--;
                   		}
                   		//blocco il processo corrente
						thread_outqueue(current_thread);
						thread_enqueue(current_thread,&wait_queue);
               		}
               		//lo uccido
				    else {
			            ssi_terminate_thread(current_thread);
					}

				break; 
			}
		// Se invece è in user mode 
		} else if((current_thread->t_s.cpsr & STATUS_USER_MODE) == STATUS_USER_MODE){
			    //se hanno un sysmgr adeguato
				    if(current_thread->t_pcb->sysMgr != NULL) {
						msgq_add(current_thread,current_thread->t_pcb->sysMgr,(uintptr_t)&(current_thread->t_s.CP15_Cause));
						//sveglio il manager
						if (out_thread(&wait_queue,current_thread->t_pcb->sysMgr)!=NULL) {
					   				 thread_enqueue(current_thread->t_pcb->sysMgr,&ready_queue);
                       				 soft_block_count--;
                   		}
                   		//blocco il processo corrente
						thread_outqueue(current_thread);
						thread_enqueue(current_thread,&wait_queue);
               		  }
               		//se  hanno un program pgr adeguato
				    else if(current_thread->t_pcb->prgMgr != NULL) {
						msgq_add(current_thread,current_thread->t_pcb->prgMgr,(unsigned int)&(current_thread->t_s.CP15_Cause));
						//sveglio il manager
						if (out_thread(&wait_queue,current_thread->t_pcb->prgMgr)!=NULL) {
							thread_enqueue(current_thread->t_pcb->prgMgr,&ready_queue);
                       		soft_block_count--;
                   		}
                   		//blocco il processo corrente
						thread_outqueue(current_thread);
						thread_enqueue(current_thread,&wait_queue);
               		}
               		//altrimenti lo uccido
				    else {
			            ssi_terminate_thread(current_thread);
					}
		}
	// Altrimenti se l'eccezione è di tipo BreakPoint 
	} else if(cause == EXC_BREAKPOINT){
		//devo fare qualcosa di particolare?
		;
	}
	//richiamo il mio amato scheduler
	scheduler();
}
