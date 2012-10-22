/* taskcom.c
*/

#include <stdlib.h>
#include <dos.h>
#include "typedef.h"
#include "inkey.h"
#include "pchw.h"
#include "task0.h"

#define _V_	/* don't care volatile to QUEUE, so compiler do optimize */
#include "comm.h"

ASSERTFILE(__FILE__)
/******************************************************************************/

static void tComm_wakeup ( TQueue * Q )
{
	unsigned long	sts	;
	TQnode		*qn, *q ;
	COMM		*sio	;

	sio = (COMM*)(Q->cargo) ;
	sts = Q->data ;
	Q->data = 0 ;

	if ( sio->inpbuf.filled == 0 ) sts &= ~TASK_RX ;
	if ( sio->outbuf.filled >= sio->outbuf.size ) sts &= ~TASK_TX ;
	if ( sts == 0 ) return ;

	qn = Q->first ;
	while ( qn != (TQnode*)Q ) {
		q = qn->next ;
		if ( qn->data & sts ) { 	/* wait events occus */
			sts &= ~qn->data ;
			qCPU_add(qn->nTask,WAKEUP_COMM) ;
			if ( sts == 0 ) break ; /* no more event */
		}
		qn = q ;
	}
}


static int tComm_wait_key ( COMM *sio )
{
	int	ch	;

	if ( (sio)->inpbuf.filled ) return WAKEUP_COMM ;

	if ( (ch=in_key()) != NOKEY ) {
		unin_key(ch) ;
		return WAKEUP_KBD ;
	}

	return 0 ;
}


static void tComm_finish ( TQueue *Q )	/* call by tqueue_free() */
{
	COMM	*sio = (COMM*)(Q->cargo) ;
	sio->_qCOMM = NULL ;
}


static void tComm_init ( COMM *sio )
{
	TQueue	*t	;

	t = tqueue_new(tComm_wakeup,tComm_finish) ;
	TASK_ASSERT(t,"TaskComm_init: memory not enough") ;

#if CODE_TYPE == CODE_PROTECTED
	_lock_data(t,sizeof(TQueue)) ;
#endif

	/* links the two objects (TQueue*) & (COMM*) */
	t->cargo = (void*)sio ;
	sio->_qCOMM = (void*)t ;
	sio->taskFlag = (int*)&(t->data) ;
}


static void taskcomm ( COMM *sio, int type, unsigned millisec )
{
	switch( type ) {
	    case TASK_INIT :
		if ( sio->_qCOMM == NULL ) tComm_init(sio) ;
		return ;

	    case TASK_FINISH :
		tqueue_free(sio->_qCOMM) ;
		return ;
	}

	if ( _curr_task == NULL || sio->_qCOMM == NULL ) { /* not in TASK */
		delay(millisec) ;
		return ;
	}

	if ( type == TASK_WDELAY || type == TASK_RDELAY ) {
		qCOMM_add(sio,type==TASK_WDELAY?TASK_TX:TASK_RX) ;
	}

	task_delay((unsigned long)millisec*128/125) ;	/* 1/1024 unit */
}

/******************************************************************************/

void qCOMM_add ( COMM *sio, int taskFlags )
{
	TQnode	*qn	;

	qn = tqueue_add((TQueue*)(sio->_qCOMM),_curr_task) ;
	qn->data = taskFlags ;
}

/******************************************************************************/

void task_comm_init ( void )
{
	COMM	*sio	;
	int	i	;

	for ( i = COM1 ; (sio=comm_port(i)) != NULL ; i++ ) {
		if ( sio->taskComm == NULL ) {
			sio->taskComm = (TaskComm)taskcomm ;
			sio->_qCOMM   = NULL ;
			sio->taskFlag = NULL ;
		}
	}
}


int comm_wait ( COMM *sio, unsigned long millisec )
{
	if ( (sio)->inpbuf.filled == 0 ) {
		if ( _curr_task && sio->_qCOMM ) {
			qCOMM_add(sio,TASK_RX) ;
			task_delay(millisec*128/125) ;
			if ( _curr_task->wakeupFlags != WAKEUP_COMM ) return -1 ;
		} else {
			delay(millisec) ;
		}
	}
	return (sio)->inpbuf.filled ;
}


int comm_wait_key ( COMM *sio, unsigned long millisec )
{
	int	sts	;

	sts = tComm_wait_key(sio) ;
	if ( sts ) return sts ;

	if ( _curr_task == NULL || sio->_qCOMM == NULL ) { /* not in TASK */
		delay(millisec) ;
		sts = tComm_wait_key(sio) ;
		return sts ? sts : WAKEUP_TIMER ;
	}

	qCOMM_add(sio,TASK_RX) ;
	qKBD_add() ;
	task_delay(millisec*128/125) ;

	return _curr_task->wakeupFlags ;
}

/******************************************************************************/
