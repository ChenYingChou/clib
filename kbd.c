/* kbd.c */

#include <stdlib.h>
#include "pchw.h"

static int ext_code ( int code )
{
	switch( code ) {
	    case 28 : return SCAN_KPD_ENTER ;
	    case 29 : return SCAN_RCTRL ;
	    case 55 : return SCAN_PRTSC ;
	    case 53 : return SCAN_KPD_SLASH ;
	    case 42 : return 0 ; /*subsection of prtsc, ignore...*/
	    case 56 : return SCAN_RALT ;
	    case 71 : return SCAN_HOME ;
	    case 72 : return SCAN_UP ;
	    case 73 : return SCAN_PGUP ;
	    case 75 : return SCAN_LEFT ;
	    case 77 : return SCAN_RIGHT ;
	    case 79 : return SCAN_END ;
	    case 80 : return SCAN_DOWN ;
	    case 81 : return SCAN_PGDN ;
	    case 82 : return SCAN_INSERT ;
	    case 83 : return SCAN_DELETE ;
	}
	return code ;
}

volatile char _kbd_pressed[105] LOCKED_DATA = { 0 } ;
volatile char _kbd_pause LOCKED_DATA = 0 ;

static	volatile int extended LOCKED_DATA = 0 ;
static	int pause LOCKED_DATA = 0 ;
static	int kbd_passon LOCKED_DATA = 1 ;
static	int (*user_handler)(int scan, int state) LOCKED_DATA = NULL ;

static int New09 ( void ) LOCKED_CODE ;
static int New09 ( void )
{
	int brk ;
	int code = inportb(0x60) ;

	if ( pause ) { // skipping subcodes of pause
		pause-- ;
		return kbd_passon == 2 ? 0 : kbd_passon ;
	}

	if ( code == 0xe0 ) {
		extended = 1 ;
		return kbd_passon ;
	}

	if ( code == 0xe1 ) { // pause pressed
		pause = 5 ; // skip 5 codes
		_kbd_pause = !_kbd_pause ;
		return kbd_passon == 2 ? 0 : kbd_passon ;
	}

	brk = code & 128 ;
	code &= 127 ;

	if ( extended ) {
		extended = 0 ;
		code = ext_code(code) ;
	}

	if ( code ) {
		_kbd_pressed[code] = !brk ;
		if ( user_handler != NULL ) return user_handler(code,!brk);
	}

	return kbd_passon;	/* return 0; ?? */
}


void kbd_install ( int (*handler)(int scan,int state), int passon )
{
	user_handler = handler ;
	kbd_passon = passon ;
	_install_irq(1,New09) ;
}


int kbd_pass ( int passon )
{
	int	old = kbd_passon ;

	if ( passon >= 0 ) kbd_passon = passon ;
	return old ;
}


void kbd_remove ( void )
{
	_remove_irq(1) ;
}

