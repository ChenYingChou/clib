/* task.c

   gcc -fomit-frame-pointer ... -c task.c

*/

#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <dos.h>
#include "typedef.h"
#include "inkey.h"
#include "pchw.h"

#define _TASK_MAIN_
#include "task0.h"

ASSERTFILE(__FILE__)
/******************************************************************************/
#define MAX_TASKS		16
#define MAX_QUEUES		4	/* assume max. queues for one task */
#define MAX_QNODES		(MAX_TASKS*MAX_QUEUES)

#if defined(CODE_PROTECTED)
#define DEFAULT_STACK		4096
#else
#define DEFAULT_STACK		1024
#endif

GLOBAL	Task		_task[MAX_TASKS]	;
GLOBAL	Task		*_curr_task		;
GLOBAL	Task		*_next_task		;

GLOBAL	int		_timer_seconds		; /* increase 1 per second */
GLOBAL	int		_kbd_reboot		; /* CTRL+ALT+DEL	   */
GLOBAL	int		_kbd_break		; /* CTRL+Break 	   */
GLOBAL	int		_kbd_ctrl_c		; /* CTRL+C		   */

static	TQnode		*_qFree0		; /* only link by *->next  */
static	TQnode		*_qFree 		; /* only link by *->next  */
static	TQueue		_qCPU			;

static	unsigned	exitSS, exitSP		;

#if defined(__WATCOMC__) && !defined(__386__)
    extern unsigned int	_STACKLOW;			/* SS:offset */
    GLOBAL unsigned int	_task_stack_size = 8*1024;
    static unsigned int	_task_stack;			/* SS:offset */
    #define ALLOC_STACK	alloc_stack
    #define FREE_STACK	free_stack
#else
    #define ALLOC_STACK	malloc
    #define FREE_STACK	free
#endif

#if defined(__WATCOMC__)
    static void interrupt FAR task_switch ( void ) ;
#else
    static void task_switch ( void ) ;
#endif

static	void task_finish0 ( void ) ;
static	int  timer_init ( void ) ;
static	int  kbd_init ( void ) ;
/******************************************************************************/

#if defined(DEBUG)
void task_abort ( const char *sFile, int nLine, const char *msg )
#else
void task_abort ( const char *msg )
#endif
{
	fflush(stdout) ;
#if defined(DEBUG)
	if ( sFile )
		fprintf(stderr,"\n\nAssertion failure in %s, line %u\n",
			sFile,nLine) ;
#endif
	fputc('\n',stderr) ;
	fputs(msg,stderr) ;
	fputs(" !\nTASK abnormal termination\n",stderr) ;
	fflush(stderr) ;
	task_finish0() ;
	exit(3) ;
}

/******************************************************************************/
#if defined(DEBUG) && defined(CHECK_LIST)
static void check_list ( void )
{
	int	nFree, nInQueue, nInTask ;
	int	i, n, err ;
	TQnode	*qn	;
	TQueue	*Q	;

	/* _curr_task in qCPU ? */
	nInTask = _curr_task - _task ;
	qn = _qCPU.first ;
	err = -1 ;
	while ( qn != (TQnode*)&_qCPU ) {
		if ( qn->nTask == nInTask ) err++ ;
		qn = qn->next ;
	}
	if ( err ) {
		fprintf(stderr,"\ncurrent task %d: %s in qCPU\n",
			nInTask,err<0?"does'nt exist":"duplicated") ;
	}

	/* How many free TQnodes ? */
	nFree = 0 ;
	qn = _qFree ;
	while ( qn ) { nFree++ ; qn = qn->next ; }

	/* How many TQnodes used by all TQueues ? */
	nInQueue = 0 ;
	Q = &_qCPU ;
	while ( Q ) {
		qn = Q->first ;
		while ( qn != (TQnode*)Q ) { nInQueue++ ; qn = qn->next ; }
		Q = Q->qLink ;
	}

	/* How many TQnodes used by all Tasks ? */
	nInTask = 0 ;
	for ( i = 0 ; i < MAX_TASKS ; i++ ) {
		qn = _task[i].tLink ;
		while ( qn ) { nInTask++ ; qn = qn->tLink ; }
	}

	if ( nInQueue != nInTask ) {
		err++ ;
		fprintf(stderr,"\nnInQueue=%d, nInTask=%d\n",nInQueue,nInTask) ;
	} else if ( nFree + nInQueue != MAX_QNODES ) {
		err++ ;
		fprintf(stderr,"\nnInQueue=%d, nFree=%d\n",nInQueue,nFree) ;
	}

	if ( err ) {
		fflush(stderr) ;
		while ( in_key() != 27 ) ;
	}
}
#endif
/******************************************************************************/
#if defined(__WATCOMC__) && !defined(__386__)

static unsigned int * get_stack_ptr ( unsigned int stack )
{
    #if defined(__MEDIUM__) || defined(__SMALL__) || defined(__TINY__)
	return (unsigned int *)(stack);
    #else
	return (unsigned int *)MK_FP(get_SS(),stack);
    #endif
}

static unsigned int * get_stack_offset_ptr ( unsigned int offset )
{
	return get_stack_ptr(_task_stack+offset);
}

static unsigned long inused_mark ( unsigned int size )
{
	return size | 0xcccc0000LU;
}

static unsigned long unused_mark ( unsigned int size )
{
	return size | 0xcdcd0000LU;
}

static int is_stack_inused ( unsigned int stack )
{
	unsigned int *p = get_stack_ptr(stack);
	if (p[1] == 0xccccU) return 1;		// in used
	TASK_ASSERT(p[1] == 0xcdcdU && p[0] <= _task_stack_size,"stack corrupted");
	return 0;	// not in used
}

static int is_stack_offset_inused ( unsigned int offset )
{
	return is_stack_inused(_task_stack+offset);
}

static void * alloc_stack ( unsigned int size )
{
	unsigned offset = 0;
	unsigned currSize;
	unsigned int *p;
	int inUsed;

	size = (size+255) & ~255;
	TASK_ASSERT(size-1 < _task_stack_size,"task alloc_stack: invalid required stack size");
	for ( ; ; ) {
		TASK_ASSERT(offset < _task_stack_size,"task alloc_stack: stack full");

		inUsed = is_stack_offset_inused(offset);
		p = get_stack_offset_ptr(offset);
		currSize = *p;

		if (!inUsed && currSize >= size) {
			currSize -= size;
			if (currSize > 0) {
				// split block: [unused:currSize] [inused:size]
				*p = currSize;
				offset += currSize;
				p = get_stack_offset_ptr(offset);
				*(unsigned long *)p = inused_mark(size);
			}
			return p;
		}

		offset += currSize;
	}
}

static void free_stack ( void *stack )
{
	unsigned int *p = get_stack_offset_ptr(0);
	unsigned int offset = (char *)stack - (char *)p;
	TASK_ASSERT(is_stack_offset_inused(offset),"task free_stack: stack not in used");
	*(unsigned long *)stack = unused_mark(*(unsigned int *)stack);

	offset = 0;
	while (offset < _task_stack_size) {
		unsigned int currSize;
		unsigned int nextOffset;
		p = get_stack_offset_ptr(offset);
		currSize = *p;
		nextOffset = offset + currSize;
		if (!is_stack_offset_inused(offset) && nextOffset < _task_stack_size) {
			unsigned int *p1 = get_stack_offset_ptr(nextOffset);
			unsigned int nextSize = *p1;
			if (!is_stack_offset_inused(nextOffset)) {
				/* merge with next block and recheck current block */
				*p = currSize + nextSize;
			} else {
				/* skip current & next blocks */
				offset = nextOffset + nextSize;
			}
		} else {
			/* skip current block */
			offset = nextOffset;
		}
	}
	TASK_ASSERT(offset==_task_stack_size,"task stack corrupted");
}

#endif
/******************************************************************************/

static void tqnode_free ( TQnode *qn )
{
	/* unlink from double links of TQueue */
	qn->next->prev = qn->prev ;
	qn->prev->next = qn->next ;
#if defined(DEBUG)
	qn->nTask = 0x1234 ;
#endif

	qn->next = _qFree ;
	_qFree = qn ;
}


static void tqnode_free_task ( Task *t )
{
	TQnode	*qn	;
	TQnode	*next	;

	qn = t->tLink ;
	while ( qn ) {
		next = qn->tLink ;
		tqnode_free(qn) ;
		qn = next ;
	}
}


static TQnode *tqnode_new ( void )
{
	TQnode	*qn	;

	TASK_ASSERT(_qFree,"tqnode_new: no free TQnode") ;
	qn = _qFree ;
	_qFree = qn->next ;
	return qn ;
}

/******************************************************************************/

TQueue *tqueue_new ( funcTQueue wFunc, funcTQueue fFunc )
{
	TQueue	*q	;

	if ( wFunc == NULL || fFunc == NULL ) return NULL ;

	q = (TQueue*)calloc(1,sizeof(TQueue)) ;
	if ( q == NULL ) return NULL ;

	q->wakeup = wFunc ;
	q->finish = fFunc ;
	q->first = (TQnode*)q ;
	q->last  = (TQnode*)q ;
	q->qLink = _qCPU.qLink ;
	_qCPU.qLink = q ;

	return q ;
}


void tqueue_free ( TQueue *Q )
{
	if ( Q ) {
		TQueue	*q0	;
		TQnode	*qn	;

		q0 = &_qCPU ;
		while ( q0 && q0->qLink != Q ) q0 = q0->qLink ;
		if ( q0 ) q0->qLink = Q->qLink ;
		(Q->finish)(Q) ;

		qn = Q->first ;
		while ( qn != (TQnode*)Q ) {
			TQnode	*next	;
			next = qn->next ;
			qCPU_add(qn->nTask,0) ; /* link to qCPU & unlink from Q */
			qn = next ;
		}
		free(Q) ;
	}
}


TQnode *tqueue_add ( TQueue *Q, Task *t )
{
	unsigned nTask	;
	TQnode	*qn	;

	nTask = t - _task ;
	TASK_ASSERT(nTask<MAX_TASKS && (t->status&TASK_ALIVE),
		    "tqueue_add: invalid task") ;

	/* remove from qCPU */
	qn = _qCPU.first ;
	while ( qn != (TQnode*)&_qCPU ) {
		if ( qn->nTask == nTask ) {
			tqnode_free(qn) ;
			t->tLink = NULL ;
			break ;
		}
		qn = qn->next ;
	}

	/* move to other TQueue (not qCPU), waiting for some events */
	t->wakeupFlags = 0 ;

	qn = tqnode_new() ;
	qn->nTask = nTask ;
	qn->data = 0 ;	/* caller should assign it -- wait for what events */

	/* link to task */
	qn->tLink = t->tLink ;
	t->tLink = qn ;

	/* link to last of Q */
	qn->next = (TQnode*)Q ;
	qn->prev = Q->last ;
	Q->last->next = qn ;
	Q->last = qn ;

	return qn ;
}

/******************************************************************************/

void qCPU_add ( unsigned nTask, unsigned who )
{
	Task	*t	;
	TQnode	*tqnode ;

	TASK_ASSERT(nTask<MAX_TASKS,"qCPU_add: task out of range") ;
	t = &_task[nTask] ;

	tqnode_free_task(t) ;
	if ( !(t->status & TASK_ALIVE) ) {
		TASK_ASSERT(t->status&TASK_STOP,"qCPU_add: task not alive") ;
		return ;
	}

	t->wakeupFlags = who ;
	tqnode = tqnode_new() ;
	t->tLink = tqnode ;

	tqnode->nTask = nTask ;
	tqnode->tLink = NULL ;
	tqnode->data  = t->priority ;

	tqnode->next = (TQnode*)&_qCPU ;
	tqnode->prev = _qCPU.last ;
	_qCPU.last->next = tqnode ;
	_qCPU.last = tqnode ;
}


static void scheduler ( void )
{
	TQueue	*Q		;
	TQnode	*qn, *qh	;
	unsigned long hiPrio	;

	/************************/
	/* Wakeup TQueue's Task */
	/************************/
	Q = _qCPU.qLink ;
	while ( Q ) {
		if ( Q->data ) (Q->wakeup)(Q) ;
		Q = Q->qLink ;
	}

/*	TASK_ASSERT(!isEmptyTQueue(&_qCPU),"scheduler: no task running") ; */

	/*******************************/
	/* Dynamic Priority Scheduling */
	/*******************************/
	hiPrio = 0 ;
	for ( qn=_qCPU.first ; qn != (TQnode*)&_qCPU ; qn = qn->next ) {
		if ( _kbd_pause && qn->data >= LOW_PRIORITY &&
				   qn->data <  MID_PRIORITY    ) continue ;
		if ( qn->data > hiPrio ) {
			qh = qn ;
			hiPrio = qh->data ;
		}
	}

	TASK_ASSERT(hiPrio,"scheduler: no task running") ;

	_next_task = &_task[qh->nTask] ;

	if ( hiPrio == LOW_PRIORITY ) { /* restore all's priority */
		qn = _qCPU.first ;
		while ( qn != (TQnode*)&_qCPU ) {
			if ( qn->data >= LOW_PRIORITY ) {
				qn->data = _task[qn->nTask].priority ;
			}
			qn = qn->next ;
		}
	} else if ( hiPrio > LOW_PRIORITY && hiPrio < HIGH_PRIORITY ) {
		qh->data-- ;
	}

	/******************************************************/
	/* Round Robin Scheduling: move (qh) to last of _qCPU */
	/******************************************************/
	if ( qh != _qCPU.last ) {
		/* unlink */
		qh->prev->next = qh->next ;
		qh->next->prev = qh->prev ;

		/* link to last of _qCPU */
		qh->next = (TQnode*)&_qCPU ;
		qh->prev = _qCPU.last ;
		_qCPU.last->next = qh ;
		_qCPU.last = qh ;
	}

	/******************/
	/* Context Switch */
	/******************/
	if ( _curr_task != _next_task ) task_switch() ;
}

/******************************************************************************/

/******************************************************************************/
/* Don't place task_switch() before scheduler(), so GCC would'nt expand it !! */
/******************************************************************************/
#if defined(__WATCOMC__)
    static void interrupt FAR task_switch ( void )
    {
	    GET_STACK(_curr_task->SS,_curr_task->SP) ;
	    _curr_task = _next_task ;
	    SET_STACK(_curr_task->SS,_curr_task->SP) ;
    }
#else
    static void task_switch ( void )
    {
	    SAVE_CONTEXT() ;
	    GET_STACK(_curr_task->SS,_curr_task->SP) ;
	    _curr_task = _next_task ;
	    SET_STACK(_curr_task->SS,_curr_task->SP) ;
	    RESTORE_CONTEXT() ;
    }
#endif


static void task0 ( int nth )	/* nth == 0 --> task[0] */
{
	NOT_USE(nth) ;
	for ( ; ; ) {
		int	i	;
		int	alive	;
		Task	*t	;

		alive = 0 ;
		t = &_task[1] ;
		for ( i = 1 ; i < MAX_TASKS ; i++, t++ ) {
			if ( t->status & TASK_STOP ) { /* free task resource */
				tqnode_free_task(t) ;
				t->tLink = NULL ;
				t->status = TASK_FREE ;
				if ( t->STACK ) {
					FREE_STACK(t->STACK) ;
					t->STACK = NULL ;
				}
			} else if ( t->status & TASK_ALIVE ) {
				alive++ ;
			}
		}

		if ( alive == 0 ) {	/* no more task, return task_start() */
			_curr_task = NULL ;
			SET_STACK(exitSS,exitSP) ;
			RESTORE_CONTEXT() ;
			RESUME0() ;
		}

		_task[0].tLink->data = 1 ; /* lowest priority */
		scheduler() ;
	}
}


static int task_get_free ( void )
{
	int	i	;

	for ( i = 0 ; i < MAX_TASKS ; i++ ) {
		if ( _task[i].status == TASK_FREE ) return i ;
	}
	return -1 ;
}


static void task_free ( Task *t )
{
	TASK_ASSERT(t!=&_task[0],"task_free: can't be task0") ;
	if ( !(t->status&TASK_ALIVE) ) return ;
	t->status = TASK_STOP ;

	if ( t == _curr_task ) {
		/* switch to task0 */
		_curr_task = &_task[0] ;
		SET_STACK(_task[0].SS,_task[0].SP) ;
		RESTORE_CONTEXT() ;
		RESUME() ;
	}

	/* force next to run, but not set priority too high(such as LONG_MAX) */
	_task[0].tLink->data = SHRT_MAX ;
}


static void task_free0 ( void ) 	/* end of task */
{
	task_free(_curr_task) ;
}

/******************************************************************************/

#if defined(__BORLANDC__) && defined(CODE_SMALL)

#include <malloc.h>

static char *TopMalloc ( size_t size )
{
	char	*pa, *pb ;
	size_t	msize = coreleft() ;

	if ( msize < size+2048 ) return malloc(size) ;
	pa = malloc(msize-size-2048) ;
	pb = malloc(size) ;
	free(pa) ;
	return pb ;
}

#endif


int task_create ( const char *name, int stackSize, unsigned prio,
		  void (*entry)(int), char *cargo )
{
static	int	xSS[4]	;
	int	n, l	;
	Task	*t	;
	int	*stk	;

	n = task_get_free() ;
	if ( n < 0 ) return -1 ;

	l = stackSize;
	if ( l < 0 )
		l = -l ;		/* not check stack size if minus */
	else if ( l < DEFAULT_STACK )
		l = DEFAULT_STACK ;
	l = (l+127) & ~127 ;

#if defined(__BORLANDC__) && defined(CODE_SMALL)
	if ( stackSize < 0 )
		stk = (int*)TopMalloc(l) ;
	else
#endif
	stk = (int*)ALLOC_STACK(l) ;
	if ( stk == NULL ) return -1 ;

	t = &_task[n] ;

	stk = (int*)((t->STACK=(char*)stk) + l) ;

	l = strlen(name) + 1 ;
	strncpy(t->name,name,l>=sizeof(t->name)?sizeof(t->name)-1:l) ;

	*--stk = n ;		/* parameter on stack */
#if defined(CODE_LARGE)
	*--stk = FP_SEG(task_free0) ;
#endif
	*--stk = FP_OFF(task_free0) ;

#if defined(__WATCOMC__)
	*--stk = get_FLAGS() ;
	*--stk = FP_SEG(entry) ;
#elif defined(CODE_LARGE)
	*--stk = FP_SEG(entry) ;
#endif
	*--stk = FP_OFF(entry) ;

	GET_STACK(xSS[0],xSS[1]) ;
	xSS[2] = FP_SEG(stk) ;
	xSS[3] = FP_OFF(stk) ;
	SET_STACK(xSS[2],xSS[3]) ;
	SAVE_CONTEXT() ;
	GET_STACK(xSS[2],xSS[3]) ;
	SET_STACK(xSS[0],xSS[1]) ;

#if defined(__WATCOMC__)
	*(stk-1) = n ;		/* parameter in register: eAX */
#endif

	t->SS = xSS[2] ;
	t->SP = xSS[3] ;

	t->tLink    = NULL ;
	t->status   = TASK_ALIVE ;
	t->priority = (prio ? prio : 50) ;
	t->cargo    = cargo ;

	qCPU_add(n,0) ;
	return n ;
}


void task_kill ( int nTask )
{
	if ( nTask <= 0 ) {	/* kill 0/myself, -1/all, -2/others */
		if ( _curr_task == NULL ) return ;
		if ( nTask < 0 ) {	/* kill all */
			int	i	;
			Task	*t	;
			for ( i=1, t=_task ; ++i < MAX_TASKS ; ) {
				if ( ++t != _curr_task ) task_free(t) ;
			}
		}
		if ( nTask >= -1 ) task_free(_curr_task) ;
	} else {
		TASK_ASSERT(nTask<MAX_TASKS,"task_kill: task out of range") ;
		task_free(&_task[nTask]) ;
	}
}


void task_pause ( void )
{
	if ( _curr_task ) scheduler() ;
}

/******************************************************************************/

void task_finish0 ( void )
{
	if ( _qCPU.qLink ) {
		TQueue	*q, *q1 ;

		q = _qCPU.qLink ;
		_qCPU.qLink = NULL ;
		while ( q ) {
			q1 = q->qLink ;
			tqueue_free(q) ;
			q = q1 ;
		}
		_qCPU.qLink = NULL ;
	}
}


int task_init ( void )
{
	TASK_ASSERT(_qCPU.first==NULL,"task_init: double execute") ;
	memset(&_task,0,sizeof(_task)) ;

#if defined(__WATCOMC__)
	_heapgrow() ;
    #if !defined(__386__)
	{
		unsigned long *p;
		_task_stack_size = (_task_stack_size+255) & ~255;
		TASK_ASSERT(stackavail() >= _task_stack_size+2048,"task_init: not enough stack size");
		_task_stack = (_STACKLOW+15) & ~15;	
		//_STACKLOW += _task_stack_size;
	    	p = (unsigned long*)get_stack_ptr(_task_stack);
		*p = unused_mark(_task_stack_size);
	}
    #endif
#endif
	{
		TQnode	*qn	;
		int	n	;

		qn = (TQnode*)calloc(MAX_QNODES,sizeof(TQnode)) ;
		if ( qn == NULL ) return -1 ;
		_qFree0 = _qFree = qn ;
		for ( n = 1 ; n < MAX_QNODES ; n++, qn++ ) {
			qn->next = qn + 1 ;
		}
		qn->next = NULL ;
	}

	_qCPU.first  = (TQnode*)&_qCPU ;
	_qCPU.last   = (TQnode*)&_qCPU ;
	_qCPU.qLink  = NULL ;

	atexit(task_finish0) ;

	if ( timer_init() < 0 || kbd_init() < 0 ) return -1 ;

	/* task0: small stack size & lowest priority */
#if defined(DEBUG) && defined(CHECK_LIST)
	return task_create("$",0,1,task0,NULL) ; /* default stack size for debug */
#elif defined(CODE_PROTECT)
	return task_create("$",-1024,1,task0,NULL) ;    /* 1K stack size */
#else
	return task_create("$",-512,1,task0,NULL) ;     /* 0.5K stack size */
#endif
}


static int task_finish ( void )
{
	if ( _task[0].STACK ) FREE_STACK(_task[0].STACK) ;

	task_finish0() ;

	if ( _qFree0 ) {
		free(_qFree0) ;
		_qFree0 = NULL ;
	}

	memset(_task,0,sizeof(_task)) ;
	memset(&_qCPU,0,sizeof(_qCPU)) ;

	TASK_ASSERT(_curr_task==NULL,"task_finish: current task not complete") ;
	return 0 ;
}


#if defined(__WATCOMC__)
    #pragma aux task_start FRAME ;
#endif
int task_start ( void )
{
#if defined(__WATCOMC__)
	if ( (int)_curr_task == 0 ) {
		RESTORE_FRAME() ;	/* #pragma aux task_start FRAME */
#else
	if ( _curr_task == 0 ) {
#endif
		/* first time */
		PUSH_FUNC(task_finish) ;
		SAVE_CONTEXT() ;
		GET_STACK(exitSS,exitSP) ;

		/* switch to task0 */
		_curr_task = &_task[0] ;
		SET_STACK(_task[0].SS,_task[0].SP) ;
		RESTORE_CONTEXT() ;
		RESUME() ;		/* -> task0(0) -> task_finish() */
	}
	return -1 ;
}

/******************************************************************************/

#define TIMER_SECOND		0x0001
#define TIMER_ALARM		0x0002

static	TQueue	*_qTIMER ;

int task_delay ( unsigned long millsec ) /* -n: alarm after n seconds */
{
	TQnode	*qn	;

	if ( _qTIMER == NULL || _curr_task == NULL ) return -1 ;

	if ( millsec ) {
		qn = tqueue_add(_qTIMER,_curr_task) ;
		qn->data = millsec ;
	}
	scheduler() ;
	return 0 ;
}


static void timer_wakeup ( TQueue * Q )
{
	unsigned long	elasped ;
	unsigned	special ;
	TQnode		*qn, *q ;
	int		expired ;

	elasped = Q->data ;
	Q->data = 0 ;

	special = (unsigned)(Q->cargo) ;
	Q->cargo = NULL ;

	qn = Q->first ;
	while ( qn != (TQnode*)Q ) {
		q = qn->next ;
		expired = 0 ;
		if ( qn->data <= elasped ) {		/* time expired */
			expired = 1 ;
		} else if ( (long)(qn->data) == LONG_MIN ) { /* timer alarm */
			if ( special & TIMER_ALARM ) expired = 1 ;
		} else if ( (long)(qn->data) < 0 ) {	/* wait seconds */
			if ( special & TIMER_SECOND ) { /* second expired */
				++(qn->data) ;
				if ( qn->data == 0 ) expired = 1 ;
			}
		} else {
			qn->data -= elasped ;
		}
		if ( expired ) qCPU_add(qn->nTask,WAKEUP_TIMER) ;
		qn = q ;
	}
}


static int timer_isr ( void ) LOCKED_CODE ;
static int timer_isr ( void )
{
	int why = cmos_why() ;

	if ( why & IRQ8_TIMER ||
	     (why & (IRQ8_ALARM|IRQ8_SECOND)) == 0 ) {
		++(_qTIMER->data) ;
	}

	if ( why & IRQ8_ALARM ) {
		_qTIMER->cargo = (void*)((unsigned)(_qTIMER->cargo)|TIMER_ALARM) ;
	}

	if ( why & IRQ8_SECOND ) {
		++_timer_seconds ;
		_qTIMER->cargo = (void*)((unsigned)(_qTIMER->cargo)|TIMER_SECOND) ;
	}

	return 0 ;
}


static void timer_finish ( TQueue *Q )
{
	NOT_USE(Q) ;	/* Q == _qTIMER */
	cmos_irq8(0) ;
	_remove_irq(8) ;
	/* unlock *(_qTIMER) when under PROTECTED mode */
	_qTIMER = NULL ;
}


static int timer_init ( void )
{
	_qTIMER = tqueue_new(timer_wakeup,timer_finish) ;
	if ( _qTIMER == NULL ) return -1 ;

#if defined(CODE_PROTECTED)
	_lock_data(_qTIMER,sizeof(TQueue)) ;
	LOCK_VARIABLE(_timer_seconds) ;
#endif
	_timer_seconds = 0 ;
	_install_irq(8,timer_isr) ;
	cmos_irq8(IRQ8_ALARM|IRQ8_TIMER|IRQ8_SECOND) ;

	return 0 ;
}

/******************************************************************************/

#define KBD_STATUS		1	/* status change		*/
#define KBD_DATA		2	/* some keys in buffer		*/

static	TQueue	*_qKBD		;
static	int	(*oldGetKey)()	;

void qKBD_add ( void )
{
	TQnode	*qn	;

	qn = tqueue_add(_qKBD,_curr_task) ;
	qn->data = KBD_DATA ;
}


int inkey ( unsigned long millsec )
{
	int	ch	;

#if defined(__DJGPP__)
	if ( _kbd_ctrl_c ) {
		_kbd_ctrl_c = 0 ;
		scan_ = SCAN_C ;
		return 3 ;
	}
#endif
	ch = in_key() ;
	if ( ch != NOKEY ) return ch ;

	if ( _qKBD == NULL || _curr_task == NULL ) return in_key() ;

	qKBD_add() ;
	task_delay(millsec) ;

	if ( _curr_task->wakeupFlags != WAKEUP_KBD ) return -1 ;
#if defined(__DJGPP__)
	if ( _kbd_ctrl_c ) {
		_kbd_ctrl_c = 0 ;
		scan_ = SCAN_C ;
		return 3 ;
	}
#endif
	return in_key();
}


static int myGetKey ( void )
{
	return inkey(0) ;
}


static void kbd_wakeup ( TQueue * Q )
{
	unsigned long	sts	;
	TQnode		*qn, *q ;
	int		ch	;

	if ( _kbd_reboot > 0 ) TASK_ABORT("kbd: reboot") ;
	if ( _kbd_break  > 0 ) TASK_ABORT("kbd: break") ;

	sts = Q->data ;
	Q->data = 0 ;
	if ( _kbd_ctrl_c )
		sts |= KBD_DATA;
	else if ( (ch=in_key()) != NOKEY ) {
		unin_key(ch) ;
		sts |= KBD_DATA ;
	}

	qn = Q->first ;
	while ( qn != (TQnode*)Q ) {
		q = qn->next ;
		if ( qn->data & sts ) { 	/* wait events occus */
			sts &= ~qn->data ;
			qCPU_add(qn->nTask,WAKEUP_KBD) ;
			if ( sts == 0 ) break ; /* no more event */
		}
		qn = q ;
	}
}


static int kbd_isr ( int scan, int isPress ) LOCKED_CODE ;
static int kbd_isr ( int scan, int isPress )
{
	if ( isPress || scan == SCAN_LALT || scan == SCAN_RALT )
		_qKBD->data |= KBD_STATUS ;

	if ( (_kbd_pressed[SCAN_LCTRL  ] || _kbd_pressed[SCAN_RCTRL  ]) &&
	     (_kbd_pressed[SCAN_LALT   ] || _kbd_pressed[SCAN_RALT   ]) &&
	     (_kbd_pressed[SCAN_DELETE ] || _kbd_pressed[SCAN_KPD_DOT])   ) {
		++_kbd_reboot ;
		return 0 ;	/* not pass to prev. keyboard isr */
	}

	if ( (_kbd_pressed[SCAN_LCTRL] || _kbd_pressed[SCAN_RCTRL]) &&
	     _kbd_pressed[SCAN_SCRLOCK] 				) {
		++_kbd_break ;
		return 0 ;	/* not pass to prev. keyboard isr */
	}

	if ( (_kbd_pressed[SCAN_LCTRL] || _kbd_pressed[SCAN_RCTRL]) &&
	     _kbd_pressed[SCAN_C]					) {
		++_kbd_ctrl_c ;
#if defined(__DJGPP__)
		return 0 ;	/* not pass to prev. keyboard isr */
#endif
	}

	return 1 ;
}


static void kbd_finish ( TQueue *Q )
{
	NOT_USE(Q) ;	/* Q == _qKBD */
	set_get_key(oldGetKey) ;
	kbd_remove() ;
	/* unlock *(_qKBD) when under PROTECTED mode */
	_qKBD = NULL ;
}


static int kbd_init ( void )
{
	oldGetKey = set_get_key(myGetKey) ;
	_qKBD = tqueue_new(kbd_wakeup,kbd_finish) ;
	if ( _qKBD == NULL ) return -1 ;

#if defined(CODE_PROTECTED)
	_lock_data(_qKBD,sizeof(TQueue)) ;
	LOCK_VARIABLE(_kbd_reboot) ;
	LOCK_VARIABLE(_kbd_break) ;
	LOCK_VARIABLE(_kbd_ctrl_c) ;
#endif
	kbd_install(kbd_isr,2) ;	/* not pass PAUSE key */

	return 0 ;
}

/******************************************************************************/
