/* task.h
*/

#ifndef _TASK_H_
#define _TASK_H_	1

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
typedef struct TQnode	TQnode		;
typedef struct TQueue	TQueue		;
typedef struct Task	Task		;
typedef void (*funcTQueue)( TQueue * )	;

struct TQnode {
	TQnode		*next		;
	TQnode		*prev		;
	unsigned long	data		;
	TQnode		*tLink		;
	int		nTask		;
} ;

struct TQueue {
	TQnode		*first		;
	TQnode		*last		;
	unsigned long	data		;
	TQueue		*qLink		;
	funcTQueue	wakeup		;
	funcTQueue	finish		;
	void		*cargo		;
} ;

struct Task {
	TQnode		*tLink		;
	char		name[16]	;
	char		*STACK		;
	unsigned	SP, SS		;
	unsigned	status		;
	unsigned	priority	;
	unsigned	wakeupFlags	;
	char		*cargo		;
} ;

enum	TaskSTS   { TASK_FREE=0, TASK_ALIVE=1, TASK_STOP=2		} ;
enum	WakeUpSTS { WAKEUP_TIMER=1, WAKEUP_KBD, WAKEUP_COMM, WAKEUP_MOUSE,
		    WAKEUP_USER 					} ;
enum	TaskPrio  { LOW_PRIORITY=11, MID_PRIORITY=5000, HIGH_PRIORITY=10000 } ;
/******************************************************************************/
extern	Task		_task[] 					;
extern	Task		*_curr_task					;
extern	Task		*_next_task					;
#if !defined(_TASK_MAIN_)
extern	volatile int	_timer_seconds					;
extern	volatile int	_kbd_reboot					;
extern	volatile int	_kbd_break					;
extern	volatile int	_kbd_ctrl_c					;
#endif
extern	volatile char	_kbd_pause		/* in kbd.c */		;
/******************************************************************************/
int	task_init   ( void )						;
int	task_start  ( void )						;
int	task_create ( const char *name, int stakeSize, unsigned prio,
		      void (*entry)(int), char *cargo ) 		;
void	task_kill   ( int nTask )					;
void	task_pause  ( void )						;
#define task_cargo()	(_curr_task->cargo)

int	task_delay  ( unsigned long millsec )	/* 1/1024 unit */	;
int	inkey	    ( unsigned long millsec )	/* 1/1024 unit */	;
/******************************************************************************/
#ifdef __cplusplus
}
#endif

#endif /* _TASK_H_ */

