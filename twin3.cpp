/*
 * $Log: TWIN3.CPP $
 * Revision 1.0  1996-02-06 23:29:13-0800  YCCHEN
 * Initial revision
 *

int twin::printf( const char *fmt, ... )				;
int xprintf( const char *fmt, ... )					;

*/

#include	<stdio.h>
#include	<stdarg.h>
#include	"twin.h"

int twin::printf ( const char *fmt, ... )
{
	int	len	;
	va_list ap	;
	char	s[160]	;

	va_start(ap,fmt) ;
	len = vsprintf(s,fmt,ap) ;
	outs(s) ;
	return len ;
}


int xprintf ( const char *fmt, ... )
{
	int	len	;
	va_list ap	;
	char	s[160]	;

	va_start(ap,fmt) ;
	len = vsprintf(s,fmt,ap) ;
	(stdscr.who())->outs(s) ;
	return len ;
}

