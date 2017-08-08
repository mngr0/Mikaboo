//Include
//test
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

/*************************************************************************************************/
/* Creazione delle quattro nuove aree nel frame riservato alla ROM  e delle variabili del nucleo */
/*******
******************************************************************************************/

void * SSI;
struct tcb_t* ttust;
struct tcb_t* ttost;
struct tcb_t* ttist;
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



/*
void sysBpHandler(){
	memaddr * base = (memaddr *) (TERM0ADDR);
	char l='0'+a0;
	*(base + 3) = 2 | (((memaddr) l) << 8);
}
*/
void tist() {
	char r='e';
	while (1){
		do_terminal_io(TERM0ADDR, DEV_TTRS_C_TRSMCHAR | (r<<8));
		//memaddr *base = (memaddr *) (TERM0ADDR);
		//*(base) = 2 | (((memaddr) 't') << 8);
	}
}

char ot,ut;
char* or, *ur;

void tust() {
	ut= 'z';
	ur="d";
	memaddr * base;
	base = (memaddr *) (TERM0ADDR);
	while (1){
		msgsend(ttost, &ut);
		ut--;
		if(ut== 'a'-1){
			ut='z';
		}
		msgrecv(ttost, &ur);
		*(base) = 2 | (((memaddr) *ur) << 8);
	}
}

void tost() {
	ot= 'A';
	or="P";
	memaddr *base = (memaddr *) (TERM0ADDR);
	while(1){
		msgrecv(ttust, &or);
		*(base) = 2 | (((memaddr) *or) << 8);
		msgsend(ttust, &ot);
		ot++;
		if(ot== 'Z'+1){
			ot='A';
		}
	}
}

extern void test();
int main() {
	init_dev_ctrl();
	currentThread=NULL;
	INIT_LIST_HEAD(&readyQueue);
	INIT_LIST_HEAD(&waitingQueue);
	INIT_LIST_HEAD(&waitForPseudoClockQueue);
    /* Settaggio delle quattro aree, ogni area:
       - imposta il PC e il registro t9 con l'address della funzione nel nucleo che deve gestire le eccezioni di questo tipo
       - imposta il $SP al RAMTOP
      - imposta il registro di stato a mascherare tutti gli interrupts, disattivare la virtual memory, e passa in kernelmode.*/

	initArea(INT_NEWAREA, (memaddr) intHandler);
	initArea(TLB_NEWAREA, (memaddr) tlbHandler);
	initArea(PGMTRAP_NEWAREA, (memaddr) pgmHandler);
	initArea(SYSBK_NEWAREA, (memaddr) sysBpHandler);
    /* Inizializzazione delle strutture dati */
	struct pcb_t *starting_process=proc_init();
	thread_init();
	msgq_init();

	//Inizializzo SSI
	SSI=thread_alloc(starting_process);
	if(SSI==NULL){
		//thread count==1
		PANIC();
	}

	//abilita interrupt e kernel mode (CHECK)
	((struct tcb_t* )SSI)->t_s.cpsr=STATUS_ALL_INT_ENABLE((((struct tcb_t* )SSI)->t_s.cpsr)|STATUS_SYS_MODE);
	//disabilita memoria virtuale
	((struct tcb_t* )SSI)->t_s.CP15_Control =CP15_DISABLE_VM (((struct tcb_t* )SSI)->t_s.CP15_Control);
	//assegno valore di CP (CHECK)(v6 forse si puo togliere)
	((struct tcb_t* )SSI)->t_s.pc=(memaddr) ssi_entry;
	//assegno valore di SP(CHECK)
	((struct tcb_t* )SSI)->t_s.sp=RAM_TOP - FRAME_SIZE ;




 /*
	//PROCESSO TEST
//	struct pcb_t* test=proc_alloc(starting_process);
	ttost=thread_alloc(starting_process);
	if (ttost==NULL){
		PANIC();
	}
	//abilita interrupt e kernel mode (CHECK)
	ttost->t_s.cpsr=STATUS_ALL_INT_ENABLE((ttost->t_s.cpsr)|STATUS_SYS_MODE);
	//disabilita memoria virtuale
	ttost->t_s.CP15_Control =CP15_DISABLE_VM (ttost->t_s.CP15_Control);
	//assegno valore di CP (CHECK)(v6 forse si puo togliere)
	ttost->t_s.pc=(memaddr) tost;
	//assegno valore di SP(CHECK)
	ttost->t_s.sp=RAM_TOP - (2*FRAME_SIZE) ;
*/



	ttust=thread_alloc(starting_process);
	if (ttust==NULL){
		PANIC();
	}

	//abilita interrupt e kernel mode (CHECK)u
	ttust->t_s.cpsr=STATUS_ALL_INT_ENABLE((ttust->t_s.cpsr)|STATUS_SYS_MODE);
	//disabilita memoria virtuale
	ttust->t_s.CP15_Control =CP15_DISABLE_VM (ttust->t_s.CP15_Control);
	//assegno valore di CP (CHECK)(v6 forse si puo togliere)
	ttust->t_s.pc=(memaddr) test;
	//assegno valore di SP(CHECK)
	ttust->t_s.sp=RAM_TOP - (3*FRAME_SIZE) ;

	thread_enqueue((struct tcb_t* )SSI,&readyQueue);
	thread_enqueue(ttust,&readyQueue);
//	thread_enqueue(ttost,&readyQueue);

	threadCount=2;

/*
	char* t= "n";
    memaddr * base;
    base = (memaddr *) (TERM0ADDR);
    *(base) = 2 | (((memaddr) *t) << 8);
*/
	scheduler();
	return 0;
}


