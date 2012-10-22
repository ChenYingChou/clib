/*
 * $Log$
 *
**/

#include	<stdlib.h>
#include	<string.h>
#include	"dbase.h"
#include	"debug.h"
ASSERTFILE(__FILE__)

double DBF::getNumber ( int nth )
{
	ASSERT((unsigned)nth < fieldCount()) ;

	if ( fieldType(nth) != 'N' ) {
		_errNo = DB_FIELD_TYPE_MISSING ;
		return -1 ;
	}

	char	s[32]	;
	return atof(gets(nth,s,sizeof(s))) ;
}


double DBF::getNumber ( const char *name )
{
	int	nth = field(name) ;
	return nth < 0 ? -1 : getNumber(nth) ;
}


int DBF::putNumber ( int nth, double val )
{
	ASSERT((unsigned)nth < fieldCount()) ;

	char	*p = fieldPtr(nth) ;
	int	l = fieldLength(nth) ;

	if ( fieldType(nth) != 'N' ) {
		_errNo = DB_FIELD_TYPE_MISSING ;
		return -1 ;
	}

	modify() ;

	char	s[32]	;
	sprintf(s,"%*.*f",fieldLength(nth),fieldDecimal(nth),val) ;
	if ( strlen(s) != fieldLength(nth) ) {	/* overflow */
		memset(p,'*',l) ;
		_errNo = DB_FIELD_OVERFLOW ;
		return -1 ;
	}

	memcpy(p,s,fieldLength(nth)) ;
	return 0 ;
}


int DBF::putNumber ( const char *name, double val )
{
	int	nth = field(name) ;
	return nth < 0 ? -1 : putNumber(nth,val) ;
}

