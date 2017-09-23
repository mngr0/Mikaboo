/*****************************************************************************
 * mikabooq.c Year 2017 v.0.1 Luglio, 15 2017                              *
 * Copyright 2017 Simone Berni, Marco Negrini, Dorotea Trestini              *
 *                                                                           *
 * This file is part of MiKABoO.                                             *
 *                                                                           *
 * MiKABoO is free software; you can redistribute it and/or modify it under  *
 * the terms of the GNU General Public License as published by the Free      *
 * Software Foundation; either version 2 of the License, or (at your option) *
 * any later version.                                                        *
 * This program is distributed in the hope that it will be useful, but       *
 * WITHOUT ANY WARRANTY; without even the implied warranty of                *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General *
 * Public License for more details.                                          *
 * You should have received a copy of the GNU General Public License along   *
 * with this program; if not, write to the Free Software Foundation, Inc.,   *
 * 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA.                  *
 *****************************************************************************/
#include "scheduler.h"
#include "const.h"
#include "exceptions.h"
//variabili locali
cpu_t slice_TOD = 0;
cpu_t last_TOD=0;

//se è terminato o meno il time slice
int is_time_slice(){
    return (SCHED_TIME_SLICE - (getTODLO() - slice_TOD) <= 0);
}
//setta il pseudoclock in ogni thread
void set_pseudo_clock(cpu_t TODLO,int time_until_slice){
    struct tcb_t *iterator=NULL;
    for_each_thread_in_q(iterator,&wait_pseudo_clock_queue){
        iterator->elapsed_time+=TODLO-last_TOD;
    }
    while((!list_empty(&wait_pseudo_clock_queue))
        &&( thread_qhead(&wait_pseudo_clock_queue)->elapsed_time>=SCHED_PSEUDO_CLOCK)) {
        struct tcb_t* thread=thread_dequeue(&wait_pseudo_clock_queue);
        if (thread->t_status==T_STATUS_READY){
            thread_enqueue(thread,&ready_queue);
            soft_block_count--;
        }
        else{
            thread_enqueue(thread,&wait_queue);
        }
        sys_send_msg(SSI,thread,(unsigned int)NULL);
    }
    int time_until_clock = SCHED_PSEUDO_CLOCK- thread_qhead(&wait_pseudo_clock_queue)->elapsed_time;
    if(time_until_slice <= time_until_clock) {
        setTIMER(time_until_slice);
    }

}

//setta il prossimo timer se è dello pseudoclock o time slice finito
void set_next_timer(){
    cpu_t TODLO = getTODLO();
    // Calcola il tempo trascorso dall'inizio del time slice corrente 
    int time_until_slice = SCHED_TIME_SLICE - (TODLO - slice_TOD);
    // Se il time slice è appena teminato setta il prossimo 
    if(time_until_slice<=0){
        slice_TOD = TODLO;
        time_until_slice= SCHED_TIME_SLICE;
    } 
    //
    if(!list_empty(&wait_pseudo_clock_queue)){
        set_pseudo_clock(TODLO,time_until_slice);  
    }
    else{
        setTIMER(time_until_slice);
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
        //in attesa di un interrupt
	   else if (thread_count > 0 && soft_block_count > 0) { 
            // impostiamo lo stato del processore con gli interrupt abilitati
            setSTATUS(STATUS_ALL_INT_ENABLE(getSTATUS()));
            WAIT();
        }
     }
     else {
        // Se non c'è nessun Thread in esecuzione ma c'e n'è almeno uno nella ready_queue allora  carico un thread
        if (current_thread == NULL) {
            current_thread = thread_dequeue(&ready_queue);
        }
        process_TOD=getTODLO();
        LDST(&(current_thread->t_s));
    }
}

