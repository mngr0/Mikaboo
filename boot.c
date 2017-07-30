#include "const.h"
#include "mikabooq.h"
#include "nucleus.h"

/*************************************************************************************************/
/* Creazione delle quattro nuove aree nel frame riservato alla ROM  e delle variabili del nucleo */
/*************************************************************************************************/

HIDDEN state_t *newAreaSysBp   = (state_t *) SYSBK_NEWAREA;
HIDDEN state_t *newAreaPgmTrap = (state_t *) PGMTRAP_NEWAREA;
HIDDEN state_t *newAreaTLBTrap = (state_t *) TLB_NEWAREA;
HIDDEN state_t *newAreaInts    = (state_t *) INT_NEWAREA;

state_t a1state, a2state, a3state;

tcb_t *tcb_SSI, *tcb_test, *tcb_init;
tcb_t *tcb_1, *tcb_2, *tcb_3;


int main() {
    
    /****************************************************************************************************************************/
    /* Settaggio delle quattro aree, ogni area:																					*/
    /*   - imposta il PC e il registro t9 con l'address della funzione nel nucleo che deve gestire le eccezioni di questo tipo	*/
    /*   - imposta il $SP al RAMTOP																								*/
    /*   - imposta il registro di stato a mascherare tutti gli interrupts, disattivare la virtual memory, e passa in kernelmode.	*/
    /****************************************************************************************************************************/

    //Specifico il gestore delle syscall e dei breakpoint
    STST(newAreaSysBp);
    newAreaSysBp->s_pc = newAreaSysBp->reg_t9 = (memaddr) sysBpHandler;
    newAreaSysBp->reg_sp = RAMTOP;
    /* Interrupt mascherati, Memoria Virtuale spenta, Kernel Mode attivo */
    newAreaSysBp->status = (newAreaSysBp->status | STATUS_KUc) & ~STATUS_INT_UNMASKED & ~STATUS_VMp;

    //specifico il gestore delle program trap
    STST(newAreaPgmTrap);
    newAreaPgmTrap->s_pc = newAreaPgmTrap->reg_t9 = (memaddr)prgHandler;
    newAreaPgmTrap->reg_sp = RAMTOP;
    newAreaPgmTrap->status = (newAreaPgmTrap->status | STATUS_KUc) & ~STATUS_INT_UNMASKED & ~STATUS_VMp;

    //specifico il gestore delle tlb trap
    STST(newAreaTLBTrap);
    newAreaTLBTrap->s_pc = newAreaTLBTrap->reg_t9 = (memaddr) tlbHandler;
    newAreaTLBTrap->reg_sp = RAMTOP;
    newAreaTLBTrap->status = (newAreaTLBTrap->status | STATUS_KUc) & ~STATUS_INT_UNMASKED & ~STATUS_VMp;

    //specifico ilgestore degli interrupt
    STST(newAreaInts);
    newAreaInts->s_pc = newAreaInts->reg_t9 = (memaddr) intsHandler;
    newAreaInts->reg_sp = RAMTOP;
    newAreaInts->status = (newAreaInts->status | STATUS_KUc) & ~STATUS_INT_UNMASKED & ~STATUS_VMp;

    /*****************************************/
    /* Inizializzazione delle strutture dati */
    
    initTcbs();
    initMsg();
    
    /* ThreadCount = 0; */
    currentThread = NULL;

    /* Allocazione del thread init per tutti i processi orfani*/
    if ((tcb_init = allocTcb()) == NULL)
        PANIC();

    /*********************************/
    /* Instanziazione del thread SSI */
    /*********************************/
    if ((tcb_SSI = allocTcb()) == NULL)
        PANIC();
    (tcb_SSI->t_state).status &= ~STATUS_VMp;
    (tcb_SSI->t_state).status &= ~STATUS_KUp; /* kernel mode */
    (tcb_SSI->t_state).status |= STATUS_IEp | STATUS_INT_UNMASKED; /* tutti gli interrupt ablilitati */
    (tcb_SSI->t_state).s_pc = (tcb_SSI->t_state).reg_t9 = (memaddr) SSI_function_entry_point;
    (tcb_SSI->t_state).reg_sp = RAMTOP - FRAME_SIZE;
    insertThread(&readyQueue, tcb_SSI);
    /* ThreadCount = 1; */


    /**********************************/
    /* Instanziazione del thread test */
    /**********************************/
    tcb_test = allocTcb();
    if ((tcb_test = allocTcb()) == NULL)
        PANIC();
    (tcb_test->t_state).status &= ~STATUS_VMp;
    (tcb_test->t_state).status &= ~STATUS_KUp;
    (tcb_test->t_state).status |= STATUS_IEp | STATUS_INT_UNMASKED;
    (tcb_test->t_state).s_pc = (tcb_test->t_state).reg_t9 = (memaddr) test;
    (tcb_test->t_state).reg_sp = RAMTOP - (2 * FRAME_SIZE);
    insertThread(&readyQueue, tcb_test);
    /* threadCount = 2; */

    /***************************/
    /* Chiamata allo scheduler */
    /***************************/

    schedule();
    
    return 0;
}
