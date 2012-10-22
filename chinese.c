/*
 * $Log: CHINESE.C $
 * Revision 1.0  1996-02-06 23:29:12-0800  YCCHEN
 * Initial revision
 *
**/

/* for Big5 only */

#include	"chinese.h"

#define CH_HIGH 	0x81
#define CH_LOW		0xfe

int is_chinese ( unsigned char c )
{
	return c >= CH_HIGH && c <= CH_LOW ;
}


int is_chinese2 ( const char s[], int nth )
{
	register unsigned char *p, *q ;

	p = (unsigned char*)s ;
	q = p + nth ;
	while ( *p != '\0' && p < q ) {
		if ( *p >= CH_HIGH && *p <= CH_LOW ) p++ ;
		p++ ;
	}
	return p != q ;
}

