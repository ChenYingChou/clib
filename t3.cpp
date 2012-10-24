/* t3.cpp
*/

#include	<stdio.h>
#include	<stdlib.h>
#include	<ctype.h>
#include	<string.h>
#include	"twin.h"
#include	"scrlay.h"
#include	"inkey.h"

#define SCREEN_NEWS		100
#define FLD_NEWS		100

static	Byte	nth		;
static	INPUT	_news[] = {
			{ FLD_NEWS	   ,	 0,	0, &nth 	    }
		}	;

/*****************************************************************************/

int main ( int argc, char *argv[] )
{
	WINSCR	*w1		;

	if ( addImage("NEWS.MSG") == 0 ) return 0 ;

	if ( argc > 1 ) nth = atoi(argv[1]) - 1 ;
	w1 = getWinScreen(SCREEN_NEWS) ;
	keepList = 2 ;
	readEdit(_news,1) ;
	closeWinScreen(w1) ;

	locate(24,1) ;
	if ( lastKey == 27 || nth >= 3 ) return 0 ;
	return nth + 1 ;
}

