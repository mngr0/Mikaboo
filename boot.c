#include "const.h"
#include "nucleus.h"
#include "arch.h"
#include "uARMconst.h"
#include "mikabooq.h"
/*************************************************************************************************/
/* Creazione delle quattro nuove aree nel frame riservato alla ROM  e delle variabili del nucleo */
/*******
******************************************************************************************/

void * SSI;



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
void tlbHandler(){

}
void pgmHandler(){

}
void sysBpHandler(){

}
void scheduler(){

}
void  ssi_entry(){

}


int main() {
 
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
	if(SSI==NULL)
		//thread count==1
		PANIC();
	//abilita interrupt e kernel mode (CHECK)
	
	((struct tcb_t* )SSI)->t_s.cpsr=STATUS_ALL_INT_ENABLE((((struct tcb_t* )SSI)->t_s.cpsr)|STATUS_SYS_MODE);
	//disabilita memoria virtuale
	((struct tcb_t* )SSI)->t_s.CP15_Control =CP15_DISABLE_VM (((struct tcb_t* )SSI)->t_s.CP15_Control);
	//assegno valore di CP (CHECK)(v6 forse si puo togliere)
	((struct tcb_t* )SSI)->t_s.pc=((struct tcb_t* )SSI)->t_s.v6=(memaddr) ssi_entry;
	//assegno valore di SP(CHECK)
	((struct tcb_t* )SSI)->t_s.sp=RAM_TOP - FRAME_SIZE ;
	scheduler();
    return 0;
}
