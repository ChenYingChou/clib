/*
 * $Log$
 *
**/

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<time.h>
#include	"dbase.h"
#include	"dbase0.h"
#if defined(UNIX)
    #include	"str.h"
#endif

int DBF::create ( const char *file, DBfield field[], int numfield )
{
	if ( _fd >= 0 ) close() ;

	if ( numfield > MAX_FIELDS ) {
		_errNo = DB_STRUC_ERR ;
		return DB_FAILURE ;
	}

#if defined(UNIX)
	int	FD = CREAT(strlwr(makeFileExt(file,".dbf"))) ;
#else
	int	FD = CREAT(file) ;
#endif
	if ( FD < 0 ) {
		_errNo = DB_CREATE_ERR ;
		return DB_FAILURE ;
	}

	int	i	;
	int	RS = 1	;
	for ( i = 0 ; i < numfield ; i++ ) {
		RS += field[i].length ;
	}

	time_t	T = time(NULL) ;
	tm	*t = localtime(&T) ;
	HEADER	h	;

	_updYear  = t->tm_year + 1900;
	_updMonth = t->tm_mon  + 1;
	_updDay   = t->tm_mday;

	memset(&h,0,sizeof(h)) ;
	h.signature	= DBF_SIGNATURE ;
	h.date[0]	= t->tm_year ;		// since 1900
	h.date[1]	= _updMonth;
	h.date[2]	= _updDay;
	h.headSize	= sizeof(HEADER) + numfield * sizeof(FIELD) + 1 ;
	h.recSize	= RS ;

	if ( WRITE(FD,&h,sizeof(h)) != sizeof(h) ) {
		CLOSE(FD) ;
		_errNo = DB_CREATE_ERR ;
		return DB_FAILURE ;
	}

	char	*F	;
	int	sz	;

	F = new char[sz=numfield*sizeof(FIELD)+1] ;
	if ( F == NULL ) {
		CLOSE(FD) ;
		_errNo = DB_NO_MEMORY ;
		return DB_FAILURE ;
	}
	memset(F,0,sz) ;

	FIELD	*f = (FIELD*)F ;
	for ( i = 0 ; i < numfield ; i++, f++ ) {
		strcpy(f->name,field[i].name) ;
		strupr(f->name) ;
		f->type     = field[i].type ;
		f->length   = field[i].length ;
		f->decimal  = field[i].decimal ;
	}

	f->name[0] = 0x0d ;
	i = WRITE(FD,F,sz) ;
	delete[] F ;
	if ( i != sz ) {
		CLOSE(FD) ;
		_errNo = DB_CREATE_ERR ;
		return DB_FAILURE ;
	}

	FLUSH(FD) ;
	_fd = FD ;

	return initial(&h) ;
}

