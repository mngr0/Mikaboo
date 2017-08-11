/*****************************************************************************
 * mikabooq.c Year 2017 v.0.1 Febbraio, 04 2017                              *
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
#include <stdlib.h>
#include "mikabooq.h"
#include "const.h"
#include <stdio.h>

struct list_head free_proc;     //free process list
struct list_head free_thread;   //free thread list
struct list_head free_msg;	//free msg list
struct pcb_t store_p[MAXPROC];	//memoria processi
struct tcb_t store_t[MAXTHREAD];//memoria thread
struct msg_t store_m[MAXMSG];	//memoria messaggi

struct pcb_t *proc_init(void){
	struct pcb_t *root=&store_p[MAXPROC-1];
	//inizializzazione root
	root->p_parent=NULL;
	INIT_LIST_HEAD(&root->p_threads);
	INIT_LIST_HEAD(&root->p_children);
	INIT_LIST_HEAD(&root->p_siblings);
	INIT_LIST_HEAD(&free_proc);
	int i; 
	//inizializzazione tutti i processi
	for(i=0;i<MAXPROC-1;i++){
		struct pcb_t* proc=&store_p[i];
		INIT_LIST_HEAD(&proc->p_siblings);
		list_add(&proc->p_siblings,&free_proc);
		INIT_LIST_HEAD(&proc->p_threads);
		INIT_LIST_HEAD(&proc->p_children);
	}
	return root;
}

struct pcb_t *proc_alloc(struct pcb_t *p_parent){
	if(p_parent==NULL)
		return NULL;
	else if(list_empty(&free_proc)){
		return NULL;
	}
	else{
		struct pcb_t* item_libero = container_of(free_proc.next,struct pcb_t,p_siblings);
		list_del(&item_libero->p_siblings);	//questo perchè la nostra list_head è attaccata ai fratelli
		INIT_LIST_HEAD(&item_libero->p_siblings);//non è più collegato ai fratelli
		INIT_LIST_HEAD(&item_libero->p_children);
		INIT_LIST_HEAD(&item_libero->p_threads);
		item_libero->p_parent=p_parent;
		list_add_tail(&item_libero->p_siblings,&p_parent->p_children);//lo aggiungo ai figli del padre
		return item_libero;
	}
}


int proc_delete(struct pcb_t *oldproc){
	if(oldproc==NULL){
		return -1;
	}
	if(!list_empty(&oldproc->p_children)){
		return -1;
	}
	else if(!list_empty(&oldproc->p_threads)){
		return -1;
	}
	else{
		list_del(&oldproc->p_siblings);
		INIT_LIST_HEAD(&oldproc->p_siblings);
	 	list_add(&oldproc->p_siblings,&free_proc); //lo aggiungo ai processi liberi
	 	oldproc->p_parent=NULL;
	 	return 0;
	 }
	}


	struct pcb_t *proc_firstchild(struct pcb_t *proc){
	struct pcb_t *p=container_of(proc->p_children.next,struct pcb_t,p_siblings); //ritorno il primo figlio
	return p;
}

struct tcb_t *proc_firstthread(struct pcb_t *proc){
	struct tcb_t *p=container_of(proc->p_threads.next,struct tcb_t,t_next); //ritorno il primo thread
	return p;
}

void thread_init(void){
	INIT_LIST_HEAD(&free_thread); //inizializzo la mia lista libera
	// e ci aggiungo tutti i thread
	int i;
	for(i=0;i<MAXTHREAD;i++){
		struct tcb_t* t=&store_t[i];
		INIT_LIST_HEAD(&t->t_next);
		list_add(&t->t_next,&free_thread);
		INIT_LIST_HEAD(&t->t_sched);
		INIT_LIST_HEAD(&t->t_msgq);
		t->t_status=0;
		t->t_pcb=NULL;
		t->t_wait4sender=NULL;
		t->start_t=0;
		t->total_t=0;
		t->exec_t=0;
	}
}

struct tcb_t *thread_alloc(struct pcb_t *process){
	//stesso ragionamento della proc_alloc
	if(process==NULL)
		return NULL;
	else if(list_empty(&free_thread)){
		return NULL;
	}
	else{
		struct tcb_t* item_libero = container_of(free_thread.next,struct tcb_t,t_next);
		list_del(&item_libero->t_next);
		INIT_LIST_HEAD(&item_libero->t_next);
		INIT_LIST_HEAD(&item_libero->t_sched);
		INIT_LIST_HEAD(&item_libero->t_msgq);
		item_libero->t_pcb=process;
		list_add_tail(&item_libero->t_next,&process->p_threads);
		return item_libero;
	}
}

int thread_free(struct tcb_t *oldthread){
	//stesso ragionamento della proc_delete
	if(oldthread==NULL){
		return -1;
	}
	if(!list_empty(&oldthread->t_msgq)){
		return -1;
	}
	else{
		list_del(&oldthread->t_next);
		INIT_LIST_HEAD(&oldthread->t_next);
		list_add(&oldthread->t_next,&free_thread);
		oldthread->t_pcb=NULL;
		return 0;
	}
}

void thread_enqueue(struct tcb_t *new, struct list_head *queue){
	if(new!=NULL && queue!=NULL){
		list_add_tail(&new->t_sched,queue); //aggiungo in coda per avere un ordine corretto quando li vado a prendere
	}
}

struct tcb_t *thread_qhead(struct list_head *queue){
	if(queue==NULL){
		return NULL;
	}else if(list_empty(queue)){
		return NULL;
	}
	else{
		struct tcb_t *p=container_of(queue->next,struct tcb_t,t_sched); //solita container of per avere l'elemento che mi serve
		return p;
	}
}
struct tcb_t *thread_dequeue(struct list_head *queue){
	if(queue==NULL){
		return NULL;
	}else if(list_empty(queue)){
		return NULL;
	}
	else{
		struct tcb_t *p=container_of(queue->next,struct tcb_t,t_sched); 
		list_del(queue->next); //vado a eliminarlo come sempre
		INIT_LIST_HEAD(&p->t_sched);
		return p;
	}
}
struct tcb_t * out_thread(struct list_head *head, struct tcb_t* this){
	if(head==NULL)
		return NULL;
	else if(list_empty(head))
		return NULL;
	else{
		struct list_head *t_temp=NULL;
		list_for_each(t_temp, head) {
       			 if( t_temp == &(this->t_next) ) {/* nel caso la trovo allora la elimino e lo restituisco */
        		    list_del( &(this->t_next) );
        		    INIT_LIST_HEAD(&(this->t_next));
            			return this;
       			 }
   		 }
   		 /* altrimenti restituisco NULL */
   		 return NULL;
	}
}
void msgq_init(void){

	int i;
	INIT_LIST_HEAD(&free_msg); //inizializzazione lista libera messaggi
	//e di tutti i suoi elementi
	for(i=0;i<MAXMSG;i++){
		struct msg_t* m= &store_m[i];
		INIT_LIST_HEAD(&m->m_next);
		list_add(&m->m_next,&free_msg);
		m->m_sender=NULL;
	}

}
int msgq_add(struct tcb_t *sender, struct tcb_t *destination,uintptr_t value){
	if(sender==NULL){
		return -1;
	}
	else if(destination==NULL){
		return -1;
	}else if(list_empty(&free_msg)){
		return -1;
	}else{
		struct msg_t* item_libero = container_of(free_msg.next,struct msg_t,m_next);
		list_del(&item_libero->m_next);
        INIT_LIST_HEAD(&item_libero->m_next);//serve per staccare item libero da ciò a cui punta 
        item_libero->m_sender=sender;
        item_libero->m_value=value;
        list_add_tail(&item_libero->m_next,&destination->t_msgq); //la add sempre in coda per avere l'ordine corretto dei messaggi
        return 0;
    }
}

int msgq_get(struct tcb_t **sender, struct tcb_t *destination,uintptr_t *value){
	if (destination==NULL){
		return -1;
	}
	else if (list_empty(&destination->t_msgq)){
		return -1;
	}
	else{
	//caso 1
		if(sender==NULL){
			struct msg_t* pm= container_of(destination->t_msgq.next,struct msg_t,m_next);
			*value=pm->m_value;
			list_del(destination->t_msgq.next);
			INIT_LIST_HEAD(&pm->m_next);
			list_add(&pm->m_next,&free_msg);
			return 0;
		}
	//caso 2
		else if (*sender==NULL){
			struct msg_t* pm= container_of(destination->t_msgq.next,struct msg_t,m_next);
			*value=pm->m_value;
			*sender=pm->m_sender;
			list_del(destination->t_msgq.next);
			INIT_LIST_HEAD(&pm->m_next);
			list_add(&pm->m_next,&free_msg);
			return 0;
		}
		//caso 3
		else{
			struct list_head* iter;
                        list_for_each(iter,&destination->t_msgq){ //ciclo per la ricerca
                        	struct msg_t* tm= container_of(iter,struct msg_t,m_next);
                        	if(*sender==tm->m_sender){
                        		*value=tm->m_value;
                        		list_del(&tm->m_next);
                        		INIT_LIST_HEAD(&tm->m_next);
                        		list_add(&tm->m_next,&free_msg);
                        		return 0;
                        	}
                        }
                        return -1;
                    }
                }
            }
