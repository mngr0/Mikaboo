#include <uARMtypes.h>
#include "scheduler.h"
#include "mikabooq.h"
#include "arch.h"
#include "nucleus.h"
#include "listx.h"

//non so perchè debbano stare qua sinceramente
unsigned int slice_TOD = 0;
unsigned int clock_TOD = 0;
unsigned int process_TOD=0;

//funzione di Marco, commenta lui
void init_dev_ctrl(){
    int i;
    for (i=0;i<DEV_USED_INTS*DEV_PER_INT;i++){
        INIT_LIST_HEAD(&device_list[i]);
    }
}

//magic is still here
int timer(unsigned int TIMER_TYPE){
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
//ancora magia, quando saprò cosa fa, la commenterò
void set_next_timer(){
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
//funzione master del file, schedula il processo giusto e controlla i deadlock
void scheduler() {
	//chiamata magica
    set_next_timer();
    //non ho piu thread nella ready queue e ho finito di eseguire il processo corrente
    if (list_empty(&ready_queue) && current_thread == NULL) {
        // è rimasto solo l ssi
        if (thread_count == 1)
            HALT(); 
        //sono in una situazione di deadlock
        else if (thread_count > 0 && soft_block_count == 0) {
            PANIC(); 
	
        }
	else if (thread_count > 0 && soft_block_count > 0) { /* in attesa di un interrupt -> wait state */
            /* se ci sono thread in attesa dello pseudo tick,
             * carico il valore dello pseudo clock nel registro della cpu.*/
            if (!list_empty(&wait_pseudo_clock_queue)) {
              //  SET_IT(SCHED_PSEUDO_CLOCK);
            }
            // impostiamo lo stato del processore con gli interrupt abilitati  //CHECK PLS
            setSTATUS(STATUS_ALL_INT_ENABLE(getSTATUS()));
		WAIT();
        }
    }
 else {
        // Se non c'è nessun Thread in esecuzione ma c'e n'è almeno uno nella ready_queue allora  carico un thread
        if (current_thread == NULL) {
            current_thread = thread_dequeue(&ready_queue);
           // current_thread->elapsedTime = 0;
           // current_thread->startTime = GET_TODLOW;
          //  SET_IT(SCHED_TIME_SLICE);
            /* Altrimenti se è passato il SCHED_TIME_SLICE rimuovo il thread corrente dall'esecuzione*/
        }// else if (current_thread->elapsedTime >= SCHED_TIME_SLICE) {
            //in questo modo do priorità all'SSI
         //   if (current_thread != tcb_SSI) {
           //     insertThread(&ready_queue, current_thread);
                /*Carico un nuovo thread*/
            //    current_thread = removeThread(&ready_queue);
	 //     }

           // current_thread->elapsedTime = 0;
           // current_thread->startTime = GET_TODLOW;

            /* Se e' scattato lo pseudo clock non settiamo il timer a 5 ms 
             * dato che scattera' subito l'interrupt dello pseudo clock */
           // if (!isPseudoClock)
           //     SET_IT(SCHED_TIME_SLICE);

//        }
        //??? magic
	   process_TOD=getTODLO();
        // carico lo stato del thread nel processore
        LDST(&(current_thread->t_s));
    }
}

