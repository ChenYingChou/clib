/* test.cpp
*/

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	"twin.h"
#include	"inkey.h"


int test ( twin &x, const char msg[], char *s[], int num )
{
#define BASE		4
	int	k, n	;
	int	toLast	;

	x.clr_scr() ;
	x.locate(2,10) ;
	x.outs(msg) ;

	for ( n = 0 ; n < num ; n++ ) {
		unin_key(13) ;
		x.get(BASE+n,10,s[n]) ;
	}

	toLast = 0 ;
	for ( n = 0 ; ; ) {
		while ( n < 0 ) n += num ;
		while ( n >= num ) n -= num ;

		k = x.get(BASE+n,10,s[n],toLast?strlen(s[n])-1:0) ;
//		if ( !x.getSecurity() ) {
//			x.locate(BASE+n,10) ;
//			x.outs(s[n]) ;
//		}
		toLast = 0 ;

		x.locate(BASE+num+1,10) ;
		x.printf("Return key = %04x",k) ;

		if ( k == 0x1b ||
		     (k == 0x0d && n+1 >= num) )
			break ;

		switch( k ) {
		    case K_UP :
		    case K_BTAB :
			n-- ;
			break ;

		    case K_HOME :
			n-- ;
			if ( x.getDoSpecial() ) unin_key(K_END) ;
			break ;

		    case K_LEFT :
		    case 0x08 :
			n-- ;
			toLast = 1 ;
			break ;

		    default :
			if ( (k > 0 && k < 127) ||
			     k == K_END 	||
			     k == K_RIGHT	||
			     k == K_DOWN		)
				n++ ;
			break ;
		}
	}
	return k == 0x1b ? -1 : 1 ;
}


main ( )
{
static	char	*SW[2] = { "Off","On " }        ;
static	char	*s[3]  = { "A...+....1....+....2....+....3....+....4"   ,
			   "B...+....1....+....2....+....3....+....4"   ,
			   "C...+....1....+....2....+....3....+....4"
			}	;
	char	msg[80] ;
	int	i	;

	for ( i = 0 ; i < 16 ; i++ ) {
		sprintf(msg,"skipBlank=%s, shiftLeft=%s, keepIns=%s, doSpecial=%s",
			SW[i&1],SW[(i>>1)&1],SW[(i>>2)&1],SW[(i>>3)&1]) ;
		stdscr.set_getSkipBlank(i&1) ;
		stdscr.set_getShiftLeft((i>>1)&1) ;
		stdscr.set_getKeepIns((i>>2)&1) ;
		stdscr.set_getDoSpecial((i>>3)&1) ;
		if ( test(stdscr,msg,s,3) < 0 ) break ;
	}

	return 0 ;
}


