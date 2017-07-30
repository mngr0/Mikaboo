#include "const.h"
#include "mikabooq.h"
#include "nucleus.h"
#include "arch.h"
#include "uARMconst.h"
/*************************************************************************************************/
/* Creazione delle quattro nuove aree nel frame riservato alla ROM  e delle variabili del nucleo */
/*************************************************************************************************/

void initArea(memaddr area, memaddr handler){
state_t *newArea = (state_t*) area;
/* Memorizza il contenuto attuale del processore in newArea */
STST(newArea);
/* Setta pc alla funzione che gestirà l'eccezione */
newArea->pc = handler;
/* Setta sp a RAMTOP */
newArea->sp = RAM_TOP;
/* Setta il registro di Stato per mascherare tutti gli interrupt e si mette in kernel-mode. */
newArea->cpsr = STATUS_ALL_INT_DISABLE((newArea->cpsr) | STATUS_SYS_MODE);
/* Disabilita la memoria virtuale */
newArea->CP15_Control =CP15_DISABLE_VM (newArea->CP15_Control);;
}

state_t a1state, a2state, a3state;

void intHandler(){
return 0;
}
void tlbHandler(){
return 0;
}
void pgmHandler(){
return 0;
}
void sysBpHandler(){
return 0;
}



int main() {
    
    /****************************************************************************************************************************/
    /* Settaggio delle quattro aree, ogni area:																					*/
    /*   - imposta il PC e il registro t9 con l'address della funzione nel nucleo che deve gestire le eccezioni di questo tipo	*/
    /*   - imposta il $SP al RAMTOP																								*/
    /*   - imposta il registro di stato a mascherare tutti gli interrupts, disattivare la virtual memory, e passa in kernelmode.	*/
    /****************************************************************************************************************************/

initArea(INT_NEWAREA, (memaddr) intHandler);
initArea(TLB_NEWAREA, (memaddr) tlbHandler);
initArea(PGMTRAP_NEWAREA, (memaddr) pgmHandler);
initArea(SYSBK_NEWAREA, (memaddr) sysBpHandler);
    /*****************************************/
    /* Inizializzazione delle strutture dati */
    return 0;
}
