

#include <uARMtypes.h>
#include "scheduler.h"
#include "mikabooq.h"
#include "arch.h"
#include "nucleus.h"//for temaddr
#include "listx.h"
/*************/
/* Scheduler */
/*************/



int softBlockCount = 0; /* il numero di thread bloccai in attesa di I/O o di completare una richiesta dal SSI */

unsigned int slice_TOD = 0;
unsigned int clock_TOD = 0;
unsigned int process_TOD=0;
void prova(){}


void init_dev_ctrl(){
    int i;
    for (i=0;i<DEV_USED_INTS*DEV_PER_INT;i++){
        INIT_LIST_HEAD(&device_list[i]);
    }
}

/**************************************************************/
/* Funzione che deve gestire clock, pseudo-clock e scheduling */

/**************************************************************/
int isTimer(unsigned int TIMER_TYPE){
    int time_until_timer;
    /* Calcola il tempo che manca allo scadere del timer di tipo TIMER_TYPE */
    if(TIMER_TYPE == SCHED_TIME_SLICE){
        time_until_timer= TIMER_TYPE - (getTODLO() - slice_TOD);
    } else if(TIMER_TYPE == SCHED_PSEUDO_CLOCK){
        time_until_timer= TIMER_TYPE - (getTODLO() - clock_TOD);
    }
    /* Se è scaduto ritorna true, altrimenti false */
    if(time_until_timer <= 0){
        return TRUE;
    } else { 
        return FALSE;
    }
}
void setNextTimer(){
    unsigned int TODLO = getTODLO();
    /* Calcola il tempo trascorso dall'inizio del time slice corrente */
    int time_until_slice = SCHED_TIME_SLICE - (TODLO - slice_TOD);

    /* Se il time slice è appena teminato setta il prossimo */
    if(time_until_slice<=0){
        slice_TOD = TODLO;
        time_until_slice= SCHED_TIME_SLICE;
    }
    
    /* Calcola il tempo trascorso dall'inizio del ciclo di pseudo clock corrente */
    int time_until_clock = SCHED_PSEUDO_CLOCK - (TODLO - clock_TOD);
    /* Se il ciclo di pseudo clock è appena teminato setta il prossimo */
    if(time_until_clock <= 0){
        clock_TOD = TODLO;
        time_until_clock = SCHED_PSEUDO_CLOCK;
    }
    /* Setta il prossimo timer */
    if(time_until_slice <= time_until_clock) {
        setTIMER(time_until_slice);
    } else {
        setTIMER(time_until_clock);
    }
}
void scheduler() {
	setNextTimer();
    /* azioni da compiere quando non c'e' nessun thread pronto ad eseguire */
    if (list_empty(&readyQueue) && currentThread == NULL) {
        prova();
        if (threadCount == 1)/* se c'e' solo il SSI -> normal system shutdown */
            HALT(); /* chiamo la HALT ROM routine */
        else if (threadCount > 0 && softBlockCount == 0) {/* deadlock */
            PANIC(); // chiamo la PANIC ROM routine 
	
        }
	 else if (threadCount > 0 && softBlockCount > 0) { /* in attesa di un interrupt -> wait state */
            /* se ci sono thread in attesa dello pseudo tick,
             * carico il valore dello pseudo clock nel registro della cpu.*/
            if (!list_empty(&waitForPseudoClockQueue)) {
              //  SET_IT(SCHED_PSEUDO_CLOCK);
            }
            /* impostiamo lo stato del processore con gli interrupt abilitati*/  //CHECK PLS
            setSTATUS(STATUS_ALL_INT_ENABLE(getSTATUS()));
		WAIT();
        }
    }
 else {
        /* Se non c'è nessun Thread in esecuzione ma c'e n'è almeno uno nella readyQueue allora 
           carico un thread*/
        if (currentThread == NULL) {
            currentThread = thread_dequeue(&readyQueue);
           // currentThread->elapsedTime = 0;
           // currentThread->startTime = GET_TODLOW;
          //  SET_IT(SCHED_TIME_SLICE);
            /* Altrimenti se è passato il SCHED_TIME_SLICE rimuovo il thread corrente dall'esecuzione*/
        }// else if (currentThread->elapsedTime >= SCHED_TIME_SLICE) {
            //in questo modo do priorità all'SSI
         //   if (currentThread != tcb_SSI) {
           //     insertThread(&readyQueue, currentThread);
                /*Carico un nuovo thread*/
            //    currentThread = removeThread(&readyQueue);
	 //     }

           // currentThread->elapsedTime = 0;
           // currentThread->startTime = GET_TODLOW;

            /* Se e' scattato lo pseudo clock non settiamo il timer a 5 ms 
             * dato che scattera' subito l'interrupt dello pseudo clock */
           // if (!isPseudoClock)
           //     SET_IT(SCHED_TIME_SLICE);

//        }
	process_TOD=getTODLO();
        /* carico lo stato del thread nel processore  dalla sua tcb */
        LDST(&(currentThread->t_s));
    }
}

