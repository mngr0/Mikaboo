	 /*******************************************************************************
  * Copyright 2014, Devid Farinelli, Erik Minarini, Alberto Nicoletti          	*
  * This file is part of kaya2014.       									    *
  *																				*
  * kaya2014 is free software: you can redistribute it and/or modify			*
  * it under the terms of the GNU General Public License as published by		*
  * the Free Software Foundation, either version 3 of the License, org          *
  * (at your option) any later version.											*
  *																				*
  * kaya2014 is distributed in the hope that it will be useful,					*
  * but WITHOUT ANY WARRANTY; without even the implied warranty of				*
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 				*
  * GNU General Public License for more details.								*
  *																				*
  * You should have received a copy of the GNU General Public licenses 			*
  * along with kaya2014.  If not, see <http://www.gnu.org/licenses/>.			*
  ******************************************************************************/

/******************************** interrupts.c **********************************
 *
 *	Questo modulo implementa la gestione delle eccezioni interrupt.
 *	Il modulo gestirà  tutti gli interrupt dei device, dell' Interval
 *  Timer e dello Pseudo Clock, convertendo gli interrupt dei device
 *	in V sui semafori appropriati.
 *
 */

#include <libuarm.h>
#include <uARMconst.h>
#include <arch.h>


#include "const.h"


#include "mikabooq.h"

#include "scheduler.h"
#include "exceptions.h"
#include "ssi.h"
#include "interrupts.h"
#include "nucleus.h"
state_t *int_old 	 = (state_t*) INT_OLDAREA;

/**
	Handler per gli interrupt.
	L'indirizzo di questa funzione è salvato nella ROM Reserved Frame
	come gestore degli interrupt.
*/

void APHERE(){}
void BPHERE(){}
void CPHERE(){}
void AAHERE(){}

struct dev_acc_ctrl* select_io_queue_from_status_addr(memaddr status_addr) {
	int dev_number =0;// DEVICE_N_FROM_REGSTATUS(status_addr);
	return &(terminal_queue[dev_number]);
	if (IS_DISK_DEVICE(status_addr)) {
		return &(disk_queue[dev_number]);
	} else if (IS_TAPE_DEVICE(status_addr)) {
		return &(tape_queue[dev_number]);
	} else if (IS_ETHERNET_DEVICE(status_addr)) {
		return &(ethernet_queue[dev_number]);
	} else if (IS_PRINTER_DEVICE(status_addr)) {
		return &(printer_queue[dev_number]);
	} else {
		return &(terminal_queue[dev_number]);
	}
}

int cause;
void intHandler(){
	int qwe=0;
	(*int_old).pc -= 4;

	if(currentThread != NULL){
		CPHERE();
		qwe=1;
		saveStateIn(int_old, &currentThread->t_s);
	}
		cause = getCAUSE();

	// Se la causa dell'interrupt è la linea 0 
		if(CAUSE_IP_GET(cause, IL_IPI)){
			lineOneTwoHandler(IL_IPI);
		} else 
	// linea 1 
		if(CAUSE_IP_GET(cause, IL_CPUTIMER)){
			lineOneTwoHandler(IL_CPUTIMER);
		} else
	//  linea 2 timer 
		if (CAUSE_IP_GET(cause, IL_TIMER)){
			//timerHandler();
		} else
    // linea 3 disk 
		if (CAUSE_IP_GET(cause, IL_DISK)){
			genericDevHandler(IL_DISK);
		} else
    // linea 4 tape
		if (CAUSE_IP_GET(cause, IL_TAPE)){
			genericDevHandler(IL_TAPE);
		} else
    // linea 5 network 
		if (CAUSE_IP_GET(cause, IL_ETHERNET)){
			genericDevHandler(IL_ETHERNET);
		} else
    // linea 6 printer
		if (CAUSE_IP_GET(cause, INT_PRINTER)){
			genericDevHandler(IL_PRINTER);
		} else
    // linea 7 terminal 
		if (CAUSE_IP_GET(cause, INT_TERMINAL)){
			terminalHandler();
		}

	scheduler();
}


/**
	@param line: Linea di Interrupt di un tipo di device
	
	Data la maschera di bit di una linea di interrupt restituisce 
	l'indice del device attivo con priorità maggiore
*/
int getHighestPriorityDev(memaddr* line){
	int activeBit = 0x00000001;
	int i;
	/* Usando una maschera (activeBit) ad ogni iterazione isolo i singoli
	bit dei device della linea. Quando ne trovo uno settato ne restituisco
	l'indice */
	for(i = 0; i < 8; i++){
		if(((*line)&activeBit) == activeBit){
			return i;
		}
		activeBit = activeBit << 1;
	}
	return -1;
}
void timerHandler(){
}
void ack(int intLine, int device, int statusReg, memaddr *commandReg){
	//memaddr *semDev 		 = getSemDev(intLine, device);
	//memaddr *kernelStatusDev = getKernelStatusDev(intLine, device);
	(*commandReg) = DEV_C_ACK;
	struct dev_acc_ctrl* q=select_io_queue_from_status_addr( intLine);
	struct tcb_t * w=thread_dequeue(&q->acc);
	w->t_s.pc -= WORD_SIZE;
	currentThread->t_status=T_STATUS_READY;
	currentThread->t_s.a1=0;
	softBlockCount--;
	msgq_add(SSI,w,(uintptr_t)statusReg);
	thread_enqueue(w,&readyQueue);
}



void lineOneTwoHandler(int interruptLineNum){

}


/**
	@param interruptLineNum: Numero della linea di interrupt

	Handler per gli interrupt di un tipo di device generico (disk, tape, network, printer)
	Manda un ACK al device ed esegue una V sul semaforo associato al device.
*/


void genericDevHandler(int interruptLineNum){
	// Uso la MACRO per ottenere la linea di interrupt 
	memaddr *intLine = (memaddr*) CDEV_BITMAP_ADDR(interruptLineNum);
	// Ottengo il device a priorità più alta 
	int device = getHighestPriorityDev(intLine);
	// Ottengo il command register del device 
	memaddr *commandReg = (memaddr*) (DEV_REG_ADDR(interruptLineNum, device) + COMMAND_REG_OFFSET);
	// Ottengo lo status register del device 
	memaddr *statusReg 	= (memaddr*) (DEV_REG_ADDR(interruptLineNum, device));
	ack(interruptLineNum, device, (*statusReg), commandReg);
}

memaddr* intLine;
void terminalHandler(){
	// Uso la MACRO per ottenere la linea di interrupt
	memaddr* intLine = (memaddr*) CDEV_BITMAP_ADDR(IL_TERMINAL);
	// Ottengo il device a priorità più alta 
	int device = getHighestPriorityDev(intLine);
	// Controllo il registro di stato del terminale per sapere se è stata effettuata una lettura o una scrittura 
	memaddr terminalRegister = (memaddr)  (DEV_REG_ADDR(IL_TERMINAL, device));
	memaddr* statusRegRead = (memaddr*) (terminalRegister + TERM_STATUS_READ);
	memaddr* commandRegRead = (memaddr*) (terminalRegister + TERM_COMMAND_READ);
	memaddr* statusRegWrite = (memaddr*) (terminalRegister + TERM_STATUS_WRITE);
	memaddr* commandRegWrite  = (memaddr*) (terminalRegister + TERM_COMMAND_WRITE);
	 
	if(((*statusRegWrite) & 0x0F) == DEV_TTRS_S_CHARTRSM){
		ack((IL_TERMINAL + 1), device, ((*statusRegWrite)), commandRegWrite);
	}

	else if(((*statusRegRead) & 0x0F) == DEV_TRCV_S_CHARRECV){
		ack(IL_TERMINAL, device, ((*statusRegRead)), commandRegRead);
	}
}
