/*
 * $Log: DEBUG.C $
 * Revision 1.1  1996-07-15 01:04:51-0700  ycchen
 * Initial revision
 *
**/

#include	<stdio.h>
#include	"debug.h"

#ifdef	DEBUG

extern	void	abort(void) ;

void _AssertMsg ( const char *sFile, int nLine )
{
	fflush(stdout) ;
	fprintf(stderr,"\n\nAssertion failure in %s, line %u\n",
		sFile,nLine) ;
	fflush(stderr) ;
	abort() ;
}

#endif
