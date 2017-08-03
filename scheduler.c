

#include <uARMtypes.h>
#include "scheduler.h"
#include "mikabooq.h"

#include "nucleus.h"//for temaddr

void scheduler(){
	
    // Setta l'interval timer al prossimo evento 
    //setNextTimer();
    // Se non c'è un processo in esecuzione 
	if(currentThread == NULL) {
        // Se la readyQueue è vuota
		if(list_empty(&readyQueue)) {
			//tprint("empty\n");
            // Se processCount è zero chiamo HALT 
			if(threadCount==0){
				HALT();
            // Se processCount > 0 e softBlockCount vale 0. Si è verificato un deadlock, invoco PANIC() 
			//}else if(processCount>0 && softBlockCount==0) {
			//	PANIC();
            // Se processCount>0 e softBlockCount>0 mi metto in stato di attesa, invoco WAIT() 
			//}else if(processCount>0 && softBlockCount>0) {
			//	setSTATUS(STATUS_ALL_INT_ENABLE(getSTATUS()));
			//	WAIT();
			}
		} else {
			currentThread = (struct tcb_t*) thread_dequeue(&readyQueue);

            // Caso anomalo 
			if(currentThread == NULL){
				PANIC();
			}
		}
	}

	// process_TOD = getTODLO();
    // Carica lo stato del processo corrente 
	//char* t= "n";
    //memaddr * base;
    //base = (memaddr *) (TERM0ADDR);
    //*(base) = 2 | (((memaddr) *t) << 8);

	LDST(&(currentThread->t_s));
}
