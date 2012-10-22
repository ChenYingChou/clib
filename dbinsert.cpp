/*
 * $Log$
 *
**/

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	"dbase.h"
#include	"dbase0.h"


int DBF::insert ( long recNum )
{
	if ( flush_data() == DB_FAILURE ) return DB_FAILURE ;

	long	pos, tot, diff	;
	UINT	bufsz, bufrec	;
	UINT	len, cnt	;
	UINT	i		;
	char	*buf		;

	if ( recNo() <= 0 || (tot=recCount()-recNo()+1) < 0 ) {
		_errNo = DB_RECORD_OUT_RANGE ;
		return DB_FAILURE ;
	}

	_errNo = DB_SUCCESS ;
	if ( _cacheSize ) {
		buf = _cachePtr ;
		bufsz = _cacheSize ;
		bufrec = bufsz / recSize() ;
		_cacheFirst = _cacheNext = 0 ;
	} else {
		buf = record() ;
		bufsz = recSize() ;
		bufrec = 1 ;
	}

	if ( recNum <= 0 ) recNum = 1 ;
	diff = recNum * recSize() ;

	pos = _headSize + recCount() * recSize() ;
	while ( tot > 0 ) {
		cnt = ( tot > bufrec ? bufrec : tot ) ;
		len = cnt * recSize() ;
		tot -= cnt ;
		pos -= len ;

		lseek(_fd,pos,SEEK_SET) ;
		if ( len != READ(_fd,buf,len) ) {
			_errNo = DB_READ_ERR ;
			break ;
		}

		lseek(_fd,pos+diff,SEEK_SET) ;
		if ( WRITE(_fd,buf,len) != len ) {
			_errNo = DB_WRITE_ERR ;
			break ;
		}
	}

	memset(buf,' ',bufsz) ;
#ifdef	PROTECT
	if ( _protectCode != 0 ) {	/* need protection */
		char	*p	;
		p = buf + 1 ;
		len = recSize() - 1 ;
		for ( i = 0 ; i < bufrec ; i++, p+=recSize() ) {
			encode(p,len,_protectCode) ;
		}
	}
#endif

	i = recNum ;
	while ( i > 0 ) {
		cnt = ( i > bufrec ? bufrec : i ) ;
		len = cnt * recSize() ;
		lseek(_fd,pos,SEEK_SET) ;
		if ( WRITE(_fd,buf,len) != len ) {
			_errNo = DB_WRITE_ERR ;
			break ;
		}

		pos += len ;
		i -= cnt ;
	}

	memset(record(),' ',recSize()) ;
	_recCount += recNum ;
	_isUpdated |= UPD_HEADER ;
	flush() ;

	return _errNo == DB_SUCCESS ? DB_SUCCESS : DB_FAILURE ;
}

