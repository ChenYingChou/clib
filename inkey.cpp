/*
 * $Log: INKEY.CPP $
 * Revision 1.0  1996-02-06 23:29:12-0800  YCCHEN
 * Initial revision
 *
**/

typedef unsigned char	Byte	;
typedef unsigned short	Word	;
typedef int (*GetFunc)(void)	;

#define NOKEY		-1

#define F1		0x3b
#define F2		0x3c
#define F3		0x3d
#define F4		0x3e
#define F5		0x3f
#define F6		0x40
#define F7		0x41
#define F8		0x42
#define F9		0x43
#define F10		0x44
#define F11		0x85
#define F12		0x86

#define S_F1		0x54
#define S_F2		0x55
#define S_F3		0x56
#define S_F4		0x57
#define S_F5		0x58
#define S_F6		0x59
#define S_F7		0x5a
#define S_F8		0x5b
#define S_F9		0x5c
#define S_F10		0x5d
#define S_F11		0x87
#define S_F12		0x88

#define C_F1		0x5e
#define C_F2		0x5f
#define C_F3		0x60
#define C_F4		0x61
#define C_F5		0x62
#define C_F6		0x63
#define C_F7		0x64
#define C_F8		0x65
#define C_F9		0x66
#define C_F10		0x67
#define C_F11		0x89
#define C_F12		0x8a

#define A_F1		0x68
#define A_F2		0x69
#define A_F3		0x6a
#define A_F4		0x6b
#define A_F5		0x6c
#define A_F6		0x6d
#define A_F7		0x6e
#define A_F8		0x6f
#define A_F9		0x70
#define A_F10		0x71
#define A_F11		0x8b
#define A_F12		0x8c

#define K_UP		0x48
#define K_DOWN		0x50
#define K_LEFT		0x4b
#define K_RIGHT 	0x4d
#define K_HOME		0x47

#define C_UP		0x8d		/* enhance key	*/
#define C_DOWN		0x91		/* enhance key	*/
#define C_LEFT		0x73
#define C_RIGHT 	0x74
#define C_HOME		0x77

#define A_UP		0x49		/* [PgUp]	*/
#define A_DOWN		0x51		/* [PgDn]	*/
#define A_LEFT		0x9b		/* enhance key	*/
#define A_RIGHT 	0x9d		/* enhance key	*/
#define A_HOME		0x97		/* no key	*/

#define K_INS		0x52
#define K_DEL		0x53
#define K_END		0x4f
#define K_CLEAR 	0x75		/* Ctrl + [End] 	*/
#define K_CANCEL	0x84		/* Ctrl + [Pg Up]	*/
#define K_EXEC		0x76		/* Ctrl + [Pg Dn]	*/
#define K_HELP		0x72		/* Ctrl + [Prt Sc]	*/
#define K_BTAB		0x0f
#define C_INS		0x92		/* Ctrl + [Ins] 	*/
#define C_DEL		0x93		/* Ctrl + [Del] 	*/

/*****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

extern	int  in_key( void )						;
extern	int  clr_key( void )						;
extern	int  get_key( void )						;
extern	int  wait_key( void )						;
extern	void unin_key( int key )					;
extern	int  status_key( void ) 					;
extern	int  initial_key( int doauto )					;
extern	int  initial_key( int doauto )					;
extern	GetFunc set_get_key( GetFunc xGet )				;

extern	unsigned char	scan_						;
extern	unsigned char	isEnhanceKey_					;

#ifdef __cplusplus
};
#endif

/*****************************************************************************/

#if defined(__WATCOMC__)

	Word Bios_key_read ( Byte func ) ;
	#pragma aux Bios_key_read = \
			"int    16h"            \
			parm [ah]		\
			value [ax]		;

	// For compatiable with Chinese System, so don't use BIOS's keyboard ptr
	//#if defined(__386__)
	//	  #include	  <i86.h>
	//	  #define	  _DOS_ 	  0
	//	  #define	  KEY_NEXT	  0x0041a
	//	  #define	  KEY_EMPTY	  0x0041c
	//
	//	  inline Bios_key_ready ( Byte func )
	//	  {
	//		  return *(Word*)MK_FP(_DOS_,KEY_NEXT) !=
	//			 *(Word*)MK_FP(_DOS_,KEY_EMPTY) ;
	//	  }
	//#else
		Word Bios_key_ready ( Byte func ) ;
		#pragma aux Bios_key_ready = \
				"int    16h"            \
				"mov    ax,0"           \
				"jz     noKey"          \
				"inc    ax"             \
			"noKey:"                        \
				parm [ah]		\
				value [ax]		;
	//#endif

	Byte Bios_key_push ( Word ch ) ;
	#pragma aux Bios_key_push = \
			"mov    ah,5"           \
			"int    16h"            \
			parm [cx]		\
			value [al]		;

#elif defined(__DJGPP__)

	#include <bios.h>
	#include <dpmi.h>

	#define Bios_key_read(func)	_bios_keybrd(func)
	#define Bios_key_ready(func)	_bios_keybrd(func)

	Bios_key_push ( Word ch )
	{
		__dpmi_regs r ;
		r.x.cx = ch ;
		r.h.ah = 5 ;
		__dpmi_int(0x16,&r) ;
		return r.h.al ;
	}

#else	/* Borland C */

	static Word Bios_key_read ( Byte func )
	{
		asm {
			mov	ah,func
			int	16h
		}
		return _AX ;
	}


	static Word Bios_key_ready ( Byte func )
	{
		asm {
			mov	ah,func
			int	16h
			mov	ax,0
			jz	exitKey
			inc	ax
		}
		    exitKey:	;
		return _AX ;
	}


	static Byte Bios_key_push ( Word kdata )
	{
		asm {
			mov	cx,kdata
			mov	ah,5
			int	16h
		}
		return _AL ;
	}

#endif

/*****************************************************************************/

static	Byte	key_tab[] = {
		F1,F2,F3,F4,F5,F6,F7,F8,F9,F10,F11,F12,
		S_F1,S_F2,S_F3,S_F4,S_F5,S_F6,S_F7,S_F8,S_F9,S_F10,S_F11,S_F12,
		C_F1,C_F2,C_F3,C_F4,C_F5,C_F6,C_F7,C_F8,C_F9,C_F10,C_F11,C_F12,
		A_F1,A_F2,A_F3,A_F4,A_F5,A_F6,A_F7,A_F8,A_F9,A_F10,A_F11,A_F12,
		K_UP,K_DOWN,K_LEFT,K_RIGHT,K_HOME,
		C_UP,C_DOWN,C_LEFT,C_RIGHT,C_HOME,
		A_UP,A_DOWN,A_LEFT,A_RIGHT,A_HOME,
		K_INS,K_DEL,K_END,K_CLEAR,K_CANCEL,
		K_EXEC,K_HELP,K_BTAB,C_INS,C_DEL,
		0
	} ;

static	int	last_key    = 0 		;
static	Byte	_key_read_  = 0 		,
		_key_sts_   = 1 		,
		_key_shift_ = 2 		;

static	GetFunc _getKey 			;

unsigned char	scan_				;
unsigned char	key_				;
unsigned char	isEnhanceKey_ = initial_key(1)	;


/*
	return = -1   : no data
		 xx00 : function key
		 00xx : ascii ( 0 ~ 255 )
*/
int in_key ( void )
{
	Byte	key	;
	int	i	;

	if ( last_key == 0 ) {	/* no push key */
		if ( Bios_key_ready(_key_sts_) == 0 ) return NOKEY ;
	} else {
#if defined(__WATCOMC__) || defined(__DJGPP__)
		i = last_key ;
		last_key = 0 ;
		if ( i != NOKEY ) return i ;
#else	/* Borland C */
		_AX = last_key ;
		last_key = 0 ;
		if ( (int)_AX != NOKEY ) return _AX ;
#endif
	}

	i = Bios_key_read(_key_read_) ;
//	if ( i == 0xdc00 ) _AX = 0x0e08 ;	// Acer 570 ?
	scan_ = i >> 8 ;
	key  = i ;
	if ( key == 0xe0 && scan_ != 0 ) key = 0 ;

	if ( scan_ == 0 || key != 0 ) return key ;

	for ( i = 0 ; key_tab[i] != 0 ; ) {
		if ( scan_ == key_tab[i++] ) return i<<8 ;
	}
	return ((Word)scan_|0x80)<<8 ;
}


int clr_key ( void )
{
	int	key = 0 ;

	last_key = 0 ;
	while ( in_key() != NOKEY ) key = 1 ;
	return key ;
}


int get_key ( void )
{
static	char	inGetKey = 0	;

	if ( inGetKey == 0 && _getKey ) {
		int	k	;

		inGetKey = 1 ;
		k = (_getKey)() ;
		inGetKey = 0 ;
		return k ;
	}

	/* force enter wait keyboard */
	if ( last_key == 0 ) last_key = NOKEY ;
	return in_key() ;
}


GetFunc set_get_key ( GetFunc xGet )
{
	GetFunc old = _getKey ;
	_getKey = xGet ;
	return old ;
}


int wait_key ( void )
{
	clr_key() ;
	return get_key() ;
}


void unin_key ( int key )
{
	if ( key != NOKEY ) last_key = key ;
}


int status_key ( void )
{
	int	key = Bios_key_read(_key_shift_) ;
	return _key_shift_ < 0x10 ? (key & 0x0f) : key ;
}


int initial_key ( int doauto )
{
	/* reset to normal keyboard functions */
	_key_read_  = 0 ;
	_key_sts_   = 1 ;
	_key_shift_ = 2 ;
	isEnhanceKey_ = 0 ;

	if ( doauto ) {
		Bios_key_push(0) ;
		while ( Bios_key_ready(1) ) {
			if ( Bios_key_read(0) == 0 ) {
				isEnhanceKey_ = 1 ;
				_key_read_  = 0x10 ;
				_key_sts_   = 0x11 ;
				_key_shift_ = 0x12 ;
			}
		}
	}

	return isEnhanceKey_ ;
}

