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

        for_each_thread_in_q(aaat_temp,&wait_pseudo_clock_queue){
            aaat_temp->elapsed_time+=TODLO-last_TOD;
        }

        while((!list_empty(&wait_pseudo_clock_queue))
            &&( thread_qhead(&wait_pseudo_clock_queue)->elapsed_time>=SCHED_PSEUDO_CLOCK)) {
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
    
        if(time_until_slice <= time_until_clock) {

            setTIMER(time_until_slice);

        } else {

            setTIMER(time_until_clock);

        }
    }

    last_TOD=TODLO;  
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
            // if(thread_in_queue(&ready_queue,SSI)){
            //     thread_outqueue(SSI);
            //     current_thread = SSI;
            // }else{
                current_thread = thread_dequeue(&ready_queue);
            // }        
        }

        process_TOD=getTODLO();

        LDST(&(current_thread->t_s));
    }
}

