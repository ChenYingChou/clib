/* task0.h */

#ifndef _TASK0_H_
#define _TASK0_H_	1

#include "task.h"
#include "debug.h"
#include "fortify.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#define CODE_SMALL		0
#define CODE_LARGE		1
#define CODE_PROTECTED		2

#if defined(__386__)
    #define CODE_TYPE		CODE_PROTECTED
#elif defined(__MEDIUM__) || defined(__LARGE__) || defined(__HUGE__)
    #define CODE_TYPE		CODE_LARGE
#else
    #define CODE_TYPE		CODE_SMALL
#endif

#if defined(__DJGPP__)

    #define SAVE_CONTEXT()		asm("pushfl; pushl %ds; pushl %es;"\
					    "pushl %fs; pushl %gs; pushal")

    #define RESTORE_CONTEXT()		asm("popal; popl %gs; popl %fs;"\
					    "popl %es; popl %ds; popfl")

    #define SET_STACK(xSS,xSP)		asm("movl %0,%%ss; movl %1,%%esp"\
					    : : "g" (xSS), "g" (xSP))

    #define GET_STACK(xSS,xSP)		{ xSS = _my_ss() ;\
					  asm("movl %%esp,%0" : "=g" (xSP)) ; }

    static inline int _my_ss ( void )
    {
	int	xSS	;
	asm volatile("xor %%eax,%%eax ; mov %%ss,%%ax" : "=a" (xSS)) ;
	return xSS ;
    }

    static inline int _my_ds ( void )
    {
	int	xDS	;
	asm volatile("xor %%eax,%%eax ; mov %%ds,%%ax" : "=a" (xDS)) ;
	return xDS ;
    }

    static inline void PUSH_FUNC ( int (*func)() )
    {
	asm("pushl %0" : : "g" (func)) ;
    }

    #define RESUME0()			asm("ret")
    #define RESUME			RESUME0
    #define FP_SEG(data)		_my_ds()
    #define FP_OFF(data)		(int)(data)

#elif defined(__WATCOMC__)

    #if defined(__386__)
	void SAVE_CONTEXT ( void ) ;
	#pragma aux SAVE_CONTEXT =\
		    "pushad"            \
		    "push   ds"         \
		    "push   es"         \
		    "push   fs"         \
		    "push   gs"         ;

	void RESTORE_CONTEXT ( void ) ;
	#pragma aux RESTORE_CONTEXT =\
		    "pop    gs"         \
		    "pop    fs"         \
		    "pop    es"         \
		    "pop    ds"         \
		    "popad"             ;

	int get_FLAGS ( void ) ;
	#pragma aux get_FLAGS =\
		    "pushfd"            \
		    "pop    eax"        \
		    value [eax] 	;

	int get_SS ( void ) ;
	#pragma aux get_SS =\
		    "xor    eax,eax"    \
		    "mov    ax,ss"      \
		    value [eax] 	;

	int get_SP ( void ) ;
	#pragma aux get_SP =\
		    value [esp] 	;

	int RESTORE_FRAME ( void ) ;
	#pragma aux RESTORE_FRAME =\
		    "mov    esp,ebp"    \
		    "pop    ebp"        ;

	void SET_STACK ( int xSS, int xSP ) ;
	#pragma aux SET_STACK =\
		    "mov    ss,ax"      \
		    "mov    esp,edx"    \
		    parm [eax] [edx]	;

	void RESUME0 ( void ) ;
	#pragma aux RESUME0 = "ret" ;

	void RESUME ( void ) ;
	#pragma aux RESUME = "iretd" ;

	void PUSH_FUNC ( int (*func)() ) ;
	#pragma aux PUSH_FUNC =\
		    "push   eax"        \
		    parm [eax]		;
    #else
	void SAVE_CONTEXT ( void ) ;
	#pragma aux SAVE_CONTEXT =\
		    "pusha"             \
		    "push   ds"         \
		    "push   es"         \
		    "push   ax"         \
		    "push   ax"         ;

	void RESTORE_CONTEXT ( void ) ;
	#pragma aux RESTORE_CONTEXT =\
		    "pop    ax"         \
		    "pop    ax"         \
		    "pop    es"         \
		    "pop    ds"         \
		    "popa"              ;

	int get_FLAGS ( void ) ;
	#pragma aux get_FLAGS =\
		    "pushf"             \
		    "pop    ax"         \
		    value [ax]		;

	int get_SS ( void ) ;
	#pragma aux get_SS =\
		    "mov    ax,ss"      \
		    value [ax]		;

	int get_SP ( void ) ;
	#pragma aux get_SP =\
		    value [sp]		;

	int RESTORE_FRAME ( void ) ;
	#pragma aux RESTORE_FRAME =\
		    "mov    sp,bp"      \
		    "pop    bp"         ;

	void SET_STACK ( int xSS, int xSP ) ;
	#pragma aux SET_STACK =\
		    "mov    ss,ax"      \
		    "mov    sp,dx"      \
		    parm [ax] [dx]	;

	void RESUME0 ( void ) ;
	#pragma aux RESUME0 = "ret" ;

	void RESUME ( void ) ;
	#pragma aux RESUME = "iret" ;

      #if CODE_TYPE == CODE_LARGE
	void PUSH_FUNC ( int (*func)() ) ;
	#pragma aux PUSH_FUNC =\
		    "push   dx"         \
		    "push   ax"         \
		    parm [dx ax]	;
      #else
	void PUSH_FUNC ( int (*func)() ) ;
	#pragma aux PUSH_FUNC =\
		    "push   ax"         \
		    parm [ax]		;
      #endif
    #endif

    #define GET_STACK(xSS,xSP)		xSS=get_SS() , xSP=get_SP()

#else /* __BORLANDC__ */

    #define SAVE_CONTEXT()		asm { pushf; push ds; push es; pusha }
    #define RESTORE_CONTEXT()		asm { popa; pop es; pop ds; popf }
    #define SET_STACK(xSS,xSP)		_SS=xSS , _SP=xSP
    #define GET_STACK(xSS,xSP)		xSS=_SS , xSP=_SP
    #define PUSH(x)			asm { push x }
  #if CODE_TYPE == CODE_LARGE
    #define RESUME0()			asm { retf }
    #define PUSH_FUNC(f)		_AX=FP_SEG(task_finish) ; PUSH(ax) ;\
					_AX=FP_OFF(task_finish) ; PUSH(ax) ;
  #else
    #define RESUME0()			asm { ret }
    #define PUSH_FUNC(f)		_AX=FP_OFF(task_finish) ; PUSH(ax) ;
  #endif
    #define RESUME			RESUME0

#endif
/******************************************************************************/
#define isEmptyTQueue(Q)	((Q)->first==(TQnode*)(Q))
TQueue *tqueue_new  ( funcTQueue wFunc, funcTQueue fFunc )		;
void	tqueue_free ( TQueue *Q )					;
TQnode *tqueue_add  ( TQueue *Q, Task *task )				;
void	qCPU_add    ( unsigned nTask, unsigned who )			;
void	qKBD_add    ( void )						;
/******************************************************************************/
#if defined(DEBUG)
    void task_abort ( const char *sFile, int nLine, const char *msg ) ;
    #define TASK_ABORT(msg)		task_abort(0,0,msg)
    #define TASK_ASSERT(expr,msg)	if ( !(expr) ) \
					task_abort(_strAssertFile,__LINE__,msg)
#else
    void task_abort ( const char *msg ) ;
    #define TASK_ABORT(msg)		task_abort(msg)
    #define TASK_ASSERT(expr,msg)	if ( !(expr) ) task_abort(msg)
#endif
/******************************************************************************/
#ifdef __cplusplus
}
#endif

#endif /* _TASK0_H_ */

