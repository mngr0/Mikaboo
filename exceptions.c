#include <libuarm.h>
#include <uARMconst.h>
#include <uARMtypes.h>
#include <arch.h>
#include "const.h"
#include "mikabooq.h"
#include "scheduler.h"
#include "nucleus.h"

state_t *tlb_old 	 = (state_t*) TLB_OLDAREA;
state_t *pgmtrap_old = (state_t*) PGMTRAP_OLDAREA;
state_t *sysbp_old 	 = (state_t*) SYSBK_OLDAREA;

void saveStateIn(state_t *from, state_t *to){
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

void tlbHandler(){
    // Se un processo è eseguito dal processore salvo lo stato nella tlb_oldarea 
	/*
	if(currentProcess != NULL){
		saveStateIn(tlb_old, &currentProcess->p_s);
	}

	useExStVec(SPECTLB);
	*/
}

void pgmHandler(){
    //se un processo è eseguito dal processore salvo lo stato nella pgmtrap_oldarea*/
	/* if(currentProcess != NULL){
		saveStateIn(pgmtrap_old, &currentProcess->sp);
	}
	useExStVec(SPECPGMT);
	*/
}

void sysBpHandler(){
	saveStateIn(sysbp_old, &currentThread->t_s);
	unsigned int cause = CAUSE_EXCCODE_GET(sysbp_old->CP15_Cause);
	unsigned int a0 = sysbp_old->a1;
	struct tcb_t * a1 =(struct tcb_t *) sysbp_old->a2;
	uintptr_t a2 = sysbp_old->a3;
	int msg_res;
	// Se l'eccezione è di tipo System call 
	if(cause==EXC_SYSCALL){
    	// Se il processo è in kernel mode gestisce adeguatamente 
		if( (currentThread->t_s.cpsr & STATUS_SYS_MODE) == STATUS_SYS_MODE){
			switch(a0){
				case 0:
					PANIC();
				//SISTEMARE CON I CASI LA SYS SEND
				case SYS_SEND:
	                //a0 contiene la costante 1 (messaggio inviato)
	                //a1 contiene l'indirizzo del thread destinatario
        			//a2 contiene il puntatore al messaggio
					msg_res=msgq_add(currentThread,a1,a2);
					if(msg_res==-1){
						currentThread->t_s.a1=-1;
					}
					if(a1->t_status==T_STATUS_W4MSG){
						if((a1->t_wait4sender==currentThread)||(a1->t_wait4sender==NULL)){
							thread_outqueue(a1);
							thread_enqueue(a1,&readyQueue);
							a1->t_s.pc -= WORD_SIZE;
							currentThread->t_status=T_STATUS_READY;
							currentThread->t_s.a1=0;
							softBlockCount--;
						}
					}
        			// Evito che rientri nel codice della syscall
					currentThread->t_s.pc += WORD_SIZE;
					LDST(&currentThread->t_s);
				break;

				case SYS_RECV:
					//a0 contiene costante 2
					//a1 contiene l'indirizzo del mittente(null==tutti)
					//a2 contiene puntatore al buffer dove regitrare il messaggio(NULL== non registrare)
					msg_res=msgq_get(&a1,currentThread,(uintptr_t *)a2);
					//in a2 viene messo il puntatore alla struttura messaggio
					if (msg_res==-1){
						thread_outqueue(currentThread);
						thread_enqueue(currentThread,&waitingQueue);
						currentThread->t_wait4sender=a1;
						currentThread->t_status=T_STATUS_W4MSG;
						currentThread=NULL;
						softBlockCount++;
					}
					else{
						//con l- istruzione sotto metto in t_s.a3 il puntatore alla struct messaggio
						currentThread->t_s.a3=a2;
						// Evito che rientri nel codice della syscall
						currentThread->t_s.a1=a1;

						currentThread->t_s.pc += WORD_SIZE;
						LDST(&currentThread->t_s);
					}
				break;

				default:
					//check TUTTI I PUNTATORI
				    if(currentThread->t_pcb->sysMgr != NULL) {
					//mittente, destinatario, motivo
					msgq_add(currentThread,currentThread->t_pcb->sysMgr,(uintptr_t)&(currentThread->t_s.CP15_Cause));
					//ottimizzare outqueue con la OUTTHREAD di amikaya + if
					if (out_thread(&waitingQueue,currentThread->t_pcb->sysMgr)) {
					//	thread_outqueue(currentThread->t_pcb->sysMgr);
                       				 thread_enqueue(currentThread->t_pcb->sysMgr,&readyQueue);
                       				 softBlockCount--;
                   			}
					thread_outqueue(currentThread);
					thread_enqueue(currentThread,&waitingQueue);
               			    }
				    else if(currentThread->t_pcb->prgMgr != NULL) {
					//mittente, destinatario, motivo
					msgq_add(currentThread,currentThread->t_pcb->prgMgr,(unsigned int)&(currentThread->t_s.CP15_Cause));
					//ottimizzare outqueue con la OUTTHREAD di amikaya + if
					if (out_thread(&waitingQueue,currentThread->t_pcb->prgMgr)) {
						thread_outqueue(currentThread->t_pcb->prgMgr);
                       				 thread_enqueue(currentThread->t_pcb->prgMgr,&readyQueue);
                       				 softBlockCount--;
                   			}
					thread_outqueue(currentThread);
					thread_enqueue(currentThread,&waitingQueue);
               			    }
					/*
				    else {
			            terminate(currentThread);
					}
					*/
				break; 
			}
		// Se invece è in user mode 
		} else if((currentThread->t_s.cpsr & STATUS_USER_MODE) == STATUS_USER_MODE){
			    if(currentThread->t_pcb->sysMgr != NULL) {
					//mittente, destinatario, motivo
					msgq_add(currentThread,currentThread->t_pcb->sysMgr,(uintptr_t)&(currentThread->t_s.CP15_Cause));
					//ottimizzare outqueue con la OUTTHREAD di amikaya + if
					if (out_thread(&waitingQueue,currentThread->t_pcb->sysMgr)) {
						thread_outqueue(currentThread->t_pcb->sysMgr);
                       				 thread_enqueue(currentThread->t_pcb->sysMgr,&readyQueue);
                       				 softBlockCount--;
                   			}
					thread_outqueue(currentThread);
					thread_enqueue(currentThread,&waitingQueue);
               			    }

				    else if(currentThread->t_pcb->prgMgr != NULL) {
					//mittente, destinatario, motivo
					msgq_add(currentThread,currentThread->t_pcb->prgMgr,(uintptr_t)&(currentThread->t_s.CP15_Cause));
					//ottimizzare outqueue con la OUTTHREAD di amikaya + if
					if (out_thread(&waitingQueue,currentThread->t_pcb->prgMgr)) {
						thread_outqueue(currentThread->t_pcb->prgMgr);
                       				 thread_enqueue(currentThread->t_pcb->prgMgr,&readyQueue);
                       				 softBlockCount--;
                   			}
					thread_outqueue(currentThread);
					thread_enqueue(currentThread,&waitingQueue);
               			    }
					/*
				    else {
			                    terminate(currentThread);
					}
					*/
		}
	// Altrimenti se l'eccezione è di tipo BreakPoint 
	} else if(cause == EXC_BREAKPOINT){
		;
	}
	//richiamo il mio amato scheduler
	scheduler();
}
