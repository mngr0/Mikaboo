#include "const.h"
#include "nucleus.h"
#include "arch.h"
#include "uARMconst.h"
#include "mikabooq.h"
#include "exceptions.h"
#include "ssi.h"
#include "scheduler.h"
#include "interrupts.h"
#include "p2test.h"

void * SSI;
void * MGRMGR;
struct tcb_t* thread_test;

struct tcb_t* ttost;
struct tcb_t* ttist;

//inizializza le aree di memoria (assioma)
void initArea(memaddr area, memaddr handler){
	state_t *newArea = (state_t*) area;
	/* Memorizza il contenuto attuale del processore in newArea */
	STST(newArea);
	/* Setta pc alla funzione che gestirà l'eccezione */
	newArea->pc = newArea->v6=handler;
	/* Setta sp a RAMTOP */
	newArea->sp = RAM_TOP;
	/* Setta il registro di Stato per mascherare tutti gli interrupt e si mette in kernel-mode. */
	newArea->cpsr = STATUS_ALL_INT_DISABLE((newArea->cpsr) | STATUS_SYS_MODE);
	/* Disabilita la memoria virtuale */
	newArea->CP15_Control =CP15_DISABLE_VM (newArea->CP15_Control);
}




void tist() {
	char r='e';
	while (1){
		do_terminal_io(TERM0ADDR, DEV_TTRS_C_TRSMCHAR | (r<<8));
		//memaddr *base = (memaddr *) (TERM0ADDR);
		// *(base) = 2 | (((memaddr) 't') << 8);
	}
}
void ADHERE(){}
void AEHERE(){}
char ot,ut;
char* or, *ur;

void tust() {
	ut= 'z';
	ur="d";
	memaddr * base = (memaddr *) (TERM0ADDR);
	while (1){
		msgsend(ttost, &ut);
		ut--;
		if(ut== 'a'-1){
			ut='z';
		}

		msgrecv(ttost, &ur);
		ADHERE();
		 *(base) = 2 | (((memaddr) *ur) << 8);
		//do_terminal_io(TERM0ADDR, DEV_TTRS_C_TRSMCHAR | (*ur << 8));
	}
}

void tost() {
	ot= 'A';
	or="P";
	memaddr *base = (memaddr *) (TERM0ADDR);
	while(1){
		msgrecv(thread_test, &or);
		AEHERE();
		//do_terminal_io(TERM0ADDR, DEV_TTRS_C_TRSMCHAR | (*or << 8));
		 *(base) = 2 | (((memaddr) *or) << 8);
		msgsend(thread_test, &ot);
		ot++;
		if(ot== 'Z'+1){
			ot='A';
		}
	}
}


//Boot del nostro programma
int main() {
	MGRMGR=1;
	init_dev_ctrl();
	current_thread=NULL;
	//Inizializzo liste 
	INIT_LIST_HEAD(&ready_queue);
	INIT_LIST_HEAD(&wait_queue);
	INIT_LIST_HEAD(&wait_pseudo_clock_queue);
    /* Settaggio delle quattro aree, ogni area:
       - imposta il pc con la funzione nel nucleo che deve gestire le eccezioni di questo tipo
       - imposta il sp al RAMTOP
      - imposta il registro di stato a mascherare tutti gli interrupts, disattivare la virtual memory, e passa in kernelmode.*/

	initArea(INT_NEWAREA, (memaddr) int_handler);
	initArea(TLB_NEWAREA, (memaddr) tlb_handler);
	initArea(PGMTRAP_NEWAREA, (memaddr) pgm_handler);
	initArea(SYSBK_NEWAREA, (memaddr) sys_bp_handler);
    // Inizializzazione della fase 1
	struct pcb_t *starting_process=proc_init();
	thread_init();
	msgq_init();

	//Inizializzo SSI
	SSI=thread_alloc(starting_process);
	if(SSI==NULL){
		//thread count==1
		PANIC();
	}

	//abilita interrupt e kernel mode
	((struct tcb_t* )SSI)->t_s.cpsr=STATUS_ALL_INT_ENABLE((((struct tcb_t* )SSI)->t_s.cpsr)|STATUS_SYS_MODE);
	//disabilita memoria virtuale
	((struct tcb_t* )SSI)->t_s.CP15_Control =CP15_DISABLE_VM (((struct tcb_t* )SSI)->t_s.CP15_Control);
	//assegno valore di pc
	((struct tcb_t* )SSI)->t_s.pc=(memaddr) ssi_entry;
	//assegno valore di sp
	((struct tcb_t* )SSI)->t_s.sp=RAM_TOP - FRAME_SIZE;


	//creo processo figlio del ssi, il mio odiato test
	 struct pcb_t* proc_test=proc_alloc(starting_process);

	thread_test=thread_alloc(proc_test);
	if (thread_test==NULL){
		PANIC();
	}

	//abilita interrupt e kernel mode (CHECK)u
	thread_test->t_s.cpsr=STATUS_ALL_INT_ENABLE((thread_test->t_s.cpsr)|STATUS_SYS_MODE);
	//disabilita memoria virtuale
	thread_test->t_s.CP15_Control =CP15_DISABLE_VM (thread_test->t_s.CP15_Control);
	//assegno valore di pc 
	thread_test->t_s.pc=(memaddr) test;
	//assegno valore di SP
	thread_test->t_s.sp=RAM_TOP - (2*FRAME_SIZE);

	thread_enqueue((struct tcb_t* )SSI,&ready_queue);
	//thread_enqueue(ttost,&ready_queue);
	thread_enqueue(thread_test,&ready_queue);
	//aggiorno il numero di thread eseguiti
	thread_count=2;
	//chiamo lo scheduler.Il controllo non tornerà mai qua.
	scheduler();
	return 0;
}


