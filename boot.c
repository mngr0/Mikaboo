#include "const.h"
#include "nucleus.h"
#include "arch.h"
#include "uARMconst.h"
#include "mikabooq.h"
#include "exceptions.h"
#include "ssi.h"
/*************************************************************************************************/
/* Creazione delle quattro nuove aree nel frame riservato alla ROM  e delle variabili del nucleo */
/*******
******************************************************************************************/
#define	TERM0ADDR	0x240


void * SSI;

struct list_head readyQueue;

struct list_head waitingQueue;

struct tcb_t* currentProcess;
int processCount =0;

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

state_t a1state, a2state, a3state;

void intHandler(){

}


/*
void sysBpHandler(){
	unsigned int cause = CAUSE_EXCCODE_GET(sysbp_old->CP15_Cause);
	unsigned int a0 = (*sysbp_old).a1;
	unsigned int a1 = (*sysbp_old).a2;
	unsigned int a2 = (*sysbp_old).a3;
	unsigned int a3 = (*sysbp_old).a4;
	tprint("a0:" + a0);
	tprint("a1:" + a1);
	tprint("a2:" + a2);
	tprint("a3:" + a3);
	memaddr * base = (memaddr *) (TERM0ADDR);
	char l='0'+a0;
	*(base + 3) = 2 | (((memaddr) l) << 8);
}
*/






int main() {

	currentProcess=NULL;
	INIT_LIST_HEAD(&readyQueue);
	INIT_LIST_HEAD(&waitingQueue);
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
	((struct tcb_t* )SSI)->t_s.pc=((struct tcb_t* )SSI)->t_s.v6=(memaddr) ssi_entry;
	//assegno valore di SP(CHECK)
	((struct tcb_t* )SSI)->t_s.sp=RAM_TOP - FRAME_SIZE ;


	//PROCESSO TEST
	struct pcb_t* test=proc_alloc(starting_process);
	struct tcb_t* ttest=thread_alloc(test);
	if (ttest==NULL){
		PANIC();
	}

	//abilita interrupt e VA TOLTO LA kernel mode (CHECK)
	//CHECK se va usato la kernel mode oppure no
	ttest->t_s.cpsr=STATUS_ALL_INT_ENABLE((ttest->t_s.cpsr)|STATUS_SYS_MODE);
	//disabilita memoria virtuale
	ttest->t_s.CP15_Control =CP15_DISABLE_VM (ttest->t_s.CP15_Control);
	//assegno valore di CP (CHECK)(v6 forse si puo togliere)
	ttest->t_s.pc=(ttest->t_s.v6)=(memaddr) test;
	//assegno valore di SP(CHECK)
	ttest->t_s.sp=RAM_TOP -(2*FRAME_SIZE) ;
	thread_enqueue(ttest,&readyQueue);
	processCount=1;
	scheduler();
	return 0;
}
