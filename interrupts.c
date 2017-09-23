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

#include "const.h"
#include "scheduler.h"
#include "exceptions.h"
#include "interrupts.h"
//variabile locale
state_t *int_old 	 = (state_t*) INT_OLDAREA;

//gestisco gli interrupt
void int_handler(){
	//devo ritornare all' istruzione precedente siccome gli interrupt vengono catturati dopo l' incremento del pc
	int_old->pc -= 4;
	if(current_thread != NULL){
		current_thread->cpu_time+=getTODLO()-process_TOD;
		save_state(int_old, &current_thread->t_s);
	}
	//guardo la causa dell' interrupt
	int cause = getCAUSE();
	//timer
	if (CAUSE_IP_GET(cause, IL_TIMER)){
		timer_handler();
	}
	//disk
	else if (CAUSE_IP_GET(cause, IL_DISK)){
		device_handler(IL_DISK);
	}
	//tape
	else if (CAUSE_IP_GET(cause, IL_TAPE)){
		device_handler(IL_TAPE);
	}
	//ethernet
	else if (CAUSE_IP_GET(cause, IL_ETHERNET)){
		device_handler(IL_ETHERNET);
	}
    //printer
	else if (CAUSE_IP_GET(cause, IL_PRINTER)){
		device_handler(IL_PRINTER);
	}
    //terminal 
	else if (CAUSE_IP_GET(cause, IL_TERMINAL)){
		terminal_handler();
	}
	else{
		PANIC();
	}
	scheduler();
}



//restituisce l'indice del device attivo con priorità maggiore
int get_priority_dev(memaddr* line){
	unsigned int activeBit = 1;
	int i;
	// Usando una maschera (activeBit) ad ogni iterazione isolo i singoli
	//bit dei device della linea. Quando ne trovo uno settato ne restituisco
	//l'indice 
	for(i = 0; i < 8; i++){
		if(((*line)&activeBit) == activeBit){
			return i;
		}
		activeBit = activeBit << 1;
	}
	return -1;
}
//se è finito il time slice, rincodo il thread
void timer_handler(){
	if (is_time_slice()){
		if(current_thread!=NULL){
				thread_enqueue(current_thread,&ready_queue);
				current_thread=NULL;	
			}
	}
	// il controllo per thread che attendono la fine dello pseudo_clock è posizionato nello scheduler
	// per avere maggiore coerenza nella misura dei tempi (in particolare nell' aggiornamento del campo elapsed_time)
	// una chiamata allo scheduler viene fatta ogni volta che ricevo un interrupt
}

//manda un segnale di acknowledge al thread che è in attesa da un device
void ack(int dev_type, int dev_numb, unsigned int status, memaddr *command_reg){
	(*command_reg) = DEV_C_ACK;
	struct tcb_t * thread_dev=ELEM_IN_DEVICE_LIST(dev_type,dev_numb);
	if(thread_dev!=NULL){
		if (thread_dev->t_status==T_STATUS_READY){
			thread_enqueue(thread_dev,&ready_queue);
			soft_block_count--;			
		}else{
			thread_enqueue(thread_dev,&wait_queue);
		}
		sys_send_msg(SSI,thread_dev,status);
		ELEM_IN_DEVICE_LIST(dev_type,dev_numb)=NULL;
	}
}



//gestisce un device generico (no terminale)
void device_handler(int dev_type){
	// Uso la MACRO per ottenere la linea di interrupt 
	memaddr *interrupt_line = (memaddr*) CDEV_BITMAP_ADDR(dev_type);
	// Ottengo il device a priorità più alta
	int dev_numb = get_priority_dev(interrupt_line);
	// Ottengo il command register del device
	memaddr *command_reg = (memaddr*) (DEV_REG_ADDR(dev_type, dev_numb) + COMMAND_REG_OFFSET);
	// Ottengo lo status register del device 
	memaddr *status_reg 	= (memaddr*) (DEV_REG_ADDR(dev_type, dev_numb));
	//mando un segnale di ack al thread in attesa
	ack(dev_type, dev_numb, (*status_reg), command_reg);
}
//gestisce il terminale
void terminal_handler(){
	// Uso la MACRO per ottenere la linea di interrupt
	memaddr* interrupt_line = (memaddr*) CDEV_BITMAP_ADDR(IL_TERMINAL);
	// Ottengo il device a priorità più alta 
	int device = get_priority_dev(interrupt_line);
	// Controllo il registro di stato del terminale per sapere se è stata effettuata una lettura o una scrittura 
	memaddr term_reg = (memaddr)  (DEV_REG_ADDR(IL_TERMINAL, device));
	memaddr* status_reg_read = (memaddr*) (term_reg + TERM_STATUS_READ);
	memaddr* command_reg_read = (memaddr*) (term_reg + TERM_COMMAND_READ);
	memaddr* status_reg_write = (memaddr*) (term_reg + TERM_STATUS_WRITE);
	memaddr* command_reg_write  = (memaddr*) (term_reg + TERM_COMMAND_WRITE);
	 //è stata fatta una scrittura
	if(((*status_reg_write) & 0x0F) == DEV_TTRS_S_CHARTRSM){
		ack((IL_TERMINAL+1), device, ((*status_reg_write)), command_reg_write);
	}
	//è stata fatta una lettura
	else if(((*status_reg_read) & 0x0F) == DEV_TRCV_S_CHARRECV){
		ack(IL_TERMINAL, device, ((*status_reg_read)), command_reg_read);
	}
}
