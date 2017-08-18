#include "scheduler.h"
#include "const.h"
unsigned int slice_TOD = 0;


void BA(){}
void BB(){}
void BC(){}
void BD(){}
void BE(){}
void BF(){}
void BG(){}
void BH(){}
//inizializzazione liste di attesa dei device
void init_dev_ctrl(){
    last_TOD=0;
    int i;
    for (i=0;i<(DEV_USED_INTS+1)*DEV_PER_INT;i++){
        INIT_LIST_HEAD(&device_list[i]);
    }
}

//magic is still here
int is_time_slice(){
    return (SCHED_TIME_SLICE - (getTODLO() - slice_TOD) <= 0);
}


int time_until_slice;
int time_until_clock;
//ancora magia, quando saprò cosa fa, la commenterò
void set_next_timer(){
    unsigned int TODLO = getTODLO();
    // Calcola il tempo trascorso dall'inizio del time slice corrente 
    time_until_slice = SCHED_TIME_SLICE - (TODLO - slice_TOD);

    // Se il time slice è appena teminato setta il prossimo 
    if(time_until_slice<=0){

        slice_TOD = TODLO;
        time_until_slice= SCHED_TIME_SLICE;
    }
    //struct tcb_t 
    struct tcb_t *aaat_temp=NULL;


    if(!list_empty(&wait_pseudo_clock_queue)){
        while((!list_empty(&wait_pseudo_clock_queue))
            &&( thread_qhead(&wait_pseudo_clock_queue)->elapsed_time>SCHED_PSEUDO_CLOCK)) {
            struct tcb_t* thread=thread_dequeue(&wait_pseudo_clock_queue);
            if (thread->t_status==T_STATUS_READY){
                thread_enqueue(thread,&ready_queue);
                soft_block_count--;
            }else{
                thread_enqueue(thread,&wait_queue);
            }
            sys_send_msg(SSI,thread,(unsigned int)NULL);
        }
    }
    // Calcola il tempo trascorso dall'inizio del ciclo di pseudo clock corrente
    if(list_empty(&wait_pseudo_clock_queue)){

        setTIMER(time_until_slice);

    }else{


        time_until_clock = SCHED_PSEUDO_CLOCK- thread_qhead(&wait_pseudo_clock_queue)->elapsed_time;
        for_each_thread_in_q(aaat_temp,&wait_pseudo_clock_queue){
            aaat_temp->elapsed_time+=TODLO-last_TOD;
        }


        if(time_until_slice <= time_until_clock) {

            setTIMER(time_until_slice);

        } else {

            setTIMER(time_until_clock);

        }
    }
    // Se il ciclo di pseudo clock è appena teminato setta il prossimo 
    // if(time_until_clock <= 0){
    //     clock_TOD = TODLO;
    //     time_until_clock = SCHED_PSEUDO_CLOCK;
    // }
    // Setta il prossimo timer 
    //if (current_thread==NULL){
        
    //}
    last_TOD=getTODLO();  
}

//funzione master del file, schedula il processo giusto e controlla i deadlock
void scheduler() {
	//setto prossimo timer
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
        //CHECK
	   else if (thread_count > 0 && soft_block_count > 0) { // in attesa di un interrupt -> wait state 
            // se ci sono thread in attesa dello pseudo tick,
            // carico il valore dello pseudo clock nel registro della cpu.
            if (!list_empty(&wait_pseudo_clock_queue)) {
              //  SET_IT(SCHED_PSEUDO_CLOCK);
            }
            // impostiamo lo stato del processore con gli interrupt abilitati
            setSTATUS(STATUS_ALL_INT_ENABLE(getSTATUS()));
            WAIT();
        }
    }
 else {
       // BD();
        // Se non c'è nessun Thread in esecuzione ma c'e n'è almeno uno nella ready_queue allora  carico un thread
        if (current_thread == NULL) {
            //BE();
            current_thread = thread_dequeue(&ready_queue);
            process_TOD=getTODLO();
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
        // carico lo stato del thread nel processore
        //  setSTATUS(STATUS_ALL_INT_ENABLE(getSTATUS()));
        LDST(&(current_thread->t_s));
    }
}

