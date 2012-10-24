/*
 * $Log: INKEY.H $
 * Revision 1.0  1996-02-06 23:29:13-0800  YCCHEN
 * Initial revision
 *
**/

#if !defined(_INKEY_H_)
#define _INKEY_H_

#define 	NOKEY		(-1)

#define 	F1		(1 <<8)
#define 	F2		(2 <<8)
#define 	F3		(3 <<8)
#define 	F4		(4 <<8)
#define 	F5		(5 <<8)
#define 	F6		(6 <<8)
#define 	F7		(7 <<8)
#define 	F8		(8 <<8)
#define 	F9		(9 <<8)
#define 	F10		(10<<8)
#define 	F11		(11<<8)
#define 	F12		(12<<8)
#define 	S_F1		(13<<8)
#define 	S_F2		(14<<8)
#define 	S_F3		(15<<8)
#define 	S_F4		(16<<8)
#define 	S_F5		(17<<8)
#define 	S_F6		(18<<8)
#define 	S_F7		(19<<8)
#define 	S_F8		(20<<8)
#define 	S_F9		(21<<8)
#define 	S_F10		(22<<8)
#define 	S_F11		(23<<8)
#define 	S_F12		(24<<8)
#define 	C_F1		(25<<8)
#define 	C_F2		(26<<8)
#define 	C_F3		(27<<8)
#define 	C_F4		(28<<8)
#define 	C_F5		(29<<8)
#define 	C_F6		(30<<8)
#define 	C_F7		(31<<8)
#define 	C_F8		(32<<8)
#define 	C_F9		(33<<8)
#define 	C_F10		(34<<8)
#define 	C_F11		(35<<8)
#define 	C_F12		(36<<8)
#define 	A_F1		(37<<8)
#define 	A_F2		(38<<8)
#define 	A_F3		(39<<8)
#define 	A_F4		(40<<8)
#define 	A_F5		(41<<8)
#define 	A_F6		(42<<8)
#define 	A_F7		(43<<8)
#define 	A_F8		(44<<8)
#define 	A_F9		(45<<8)
#define 	A_F10		(46<<8)
#define 	A_F11		(47<<8)
#define 	A_F12		(48<<8)
#define 	K_UP		(49<<8)
#define 	K_DOWN		(50<<8)
#define 	K_LEFT		(51<<8)
#define 	K_RIGHT 	(52<<8)
#define 	K_HOME		(53<<8)
#define 	C_UP		(54<<8)
#define 	C_DOWN		(55<<8)
#define 	C_LEFT		(56<<8)
#define 	C_RIGHT 	(57<<8)
#define 	C_HOME		(58<<8)
#define 	K_PGUP		(59<<8)
#define 	K_PGDN		(60<<8)
#define 	A_LEFT		(61<<8)
#define 	A_RIGHT 	(62<<8)
#define 	A_HOME		(63<<8)
#define 	K_INS		(64<<8)
#define 	K_DEL		(65<<8)
#define 	K_END		(66<<8)
#define 	C_END		(67<<8)
#define 	C_PGUP		(68<<8)
#define 	C_PGDN		(69<<8)
#define 	C_PRTSCR	(70<<8)
#define 	K_BTAB		(71<<8)
#define 	C_INS		(72<<8)
#define 	C_DEL		(73<<8)

#define 	K_CLEAR 	C_END
#define 	K_CANCEL	C_PGUP
#define 	K_EXEC		C_PGDN
#define 	K_HELP		C_PRTSCR

/* define status bits */
#define 	KBS_SHIFT_RIGHT 	0x0001
#define 	KBS_SHIFT_LEFT		0x0002
#define 	KBS_CTRL		0x0004
#define 	KBS_ALT 		0x0008
#define 	KBS_SCROLL_LOCK 	0x0010
#define 	KBS_NUM_LOCK		0x0020
#define 	KBS_CAP_LOCK		0x0040
#define 	KBS_INSERT		0x0080

/* define extended status bits */
#define 	KBS_CTRL_LEFT_PRESSED	0x0100
#define 	KBS_ALT_LEFT_PRESSED	0x0200
#define 	KBS_CTRL_RIGHT_PRESSED	0x0400
#define 	KBS_ALT_RIGHT_PRESSED	0x0800
#define 	KBS_SCROLL_LOCK_PRESSED 0x1000
#define 	KBS_NUM_LOCK_PRESSED	0x2000
#define 	KBS_CAP_LOCK_PRESSED	0x4000
#define 	KBS_SYS_REQ_PRESSED	0x8000

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*GetFunc)(void) ;
GetFunc set_get_key( GetFunc xGet )					;

int	in_key( void )							;
int	clr_key( void ) 						;
int	get_key( void ) 						;
int	wait_key( void )						;
void	unin_key( int key )						;
int	status_key( void )						;
#ifdef __cplusplus
int	initial_key( int doauto = 1 )					;
#else
int	initial_key( int doauto )					;
#endif

extern	unsigned char	scan_						;
extern	unsigned char	isEnhanceKey_					;

#ifdef __cplusplus
};
#endif

#endif	/* _INKEY_H_ */

