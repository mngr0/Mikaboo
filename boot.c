#include "const.h"
#include "exceptions.h"
#include "ssi.h"
#include "scheduler.h"
#include "interrupts.h"
#include "p2test.h"
#include "boot.h"

void * SSI;


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

//Boot del nostro programma
int main() {
	//inizializzazione code device
	init_dev_ctrl();
	current_thread=NULL;
	//Inizializzo liste 
	INIT_LIST_HEAD(&ready_queue);
	INIT_LIST_HEAD(&wait_queue);
	INIT_LIST_HEAD(&wait_pseudo_clock_queue);
	//inizializzo le 4 aree di uarm
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


	//creo processo figlio del ssi
	struct pcb_t* proc_test=proc_alloc(starting_process);

	struct tcb_t* thread_test=thread_alloc(proc_test);
	if (thread_test==NULL){
		PANIC();
	}

	//abilita interrupt e kernel mode 
	thread_test->t_s.cpsr=STATUS_ALL_INT_ENABLE((thread_test->t_s.cpsr)|STATUS_SYS_MODE);
	//disabilita memoria virtuale
	thread_test->t_s.CP15_Control =CP15_DISABLE_VM (thread_test->t_s.CP15_Control);
	//assegno valore di pc 
	thread_test->t_s.pc=(memaddr) test;
	//assegno valore di SP
	thread_test->t_s.sp=RAM_TOP - (2*FRAME_SIZE);

	//incodo i thread nella ready queue
	thread_enqueue((struct tcb_t* )SSI,&ready_queue);
	thread_enqueue(thread_test,&ready_queue);
	//aggiorno il numero di thread eseguiti
	thread_count=2;
	//chiamo lo scheduler.
	scheduler();
	//Il controllo non tornerà mai qua.
	return 0;
}


