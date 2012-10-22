// $log: doall.c $
//

#include <stdlib.h>
#include "doall.h"

#define ATTR		(_A_RDONLY|_A_HIDDEN|_A_SYSTEM|_A_SUBDIR)

/*****************************************************************************/

int do_all ( const char *filespec, doallFunc func, unsigned attrib )
{
	int	i		;
	char	s[_MAX_PATH+8]	;
	char	c, *p, *q	;
	const char *r		;
	struct find_t x 	;

	if ( (i=_dos_findfirst(filespec,attrib?attrib:ATTR,&x)) != 0 ) return i ;
	p = q = s ; r = filespec ;
	while ( (c=(*q++)=(*r++)) != 0 ) {
		if ( c == '\\' || c == '/' || c == ':' ) p = q ;
	}

	do {
		if ( x.name[0] != '.' ) {
			q = p ;  r = x.name ;
			while ( (*q++ = *r++) != 0 ) ;
			(*func)(s,&x) ;
		}
	} while ( _dos_findnext(&x) == 0 ) ;
	return 0 ;
}

