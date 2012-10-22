/*
 * $Log$
 *
**/

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	"dbase.h"
#include	"dbase0.h"
#include	"debug.h"
ASSERTFILE(__FILE__)


int DBF::pack ( )
{
	if ( flush_data() == DB_FAILURE ) return DB_FAILURE ;

	long	rpos, wpos, tot ;
	UINT	bufrec		;
	UINT	len, cnt, l	;
	UINT	i		;
	char	*buf, *p, *q, *r;

	_errNo = DB_SUCCESS ;
	if ( _cacheSize ) {
		buf = _cachePtr ;
		bufrec = _cacheSize / recSize() ;
		_cacheFirst = _cacheNext = 0 ;
	} else {
		buf = record() ;
		bufrec = 1 ;
	}

	tot = recCount() ;
	rpos = wpos = _headSize ;
	while ( tot > 0 ) {
		cnt = ( tot > bufrec ? bufrec : tot ) ;
		len = cnt * recSize() ;
		tot -= cnt ;

		lseek(_fd,rpos,SEEK_SET) ;
		l = READ(_fd,buf,len) ;
		if ( l != len ) {
			_errNo = DB_READ_ERR ;
			len = l ;
			cnt = len / recSize() ;
			if ( cnt == 0 ) break ;
		}

		p = q = buf ;
		r = NULL ;
		for ( i = 0 ; i < cnt ; i++, p+=recSize() ) {
			if ( *p == '*' ) {
				if ( r == NULL ) r = p ;
			} else {
				if ( p != q ) memcpy(q,p,recSize()) ;
				q += recSize() ;
			}
		}

		//   |--相同--|--移動--|--刪除--|
		//   buf      r        q	p

		if ( rpos != wpos || p != q ) {
			if ( rpos != wpos || r == NULL )
				r = buf ;
			else
				wpos += r - buf ;

			i = q - r ;
			if ( i > 0 ) {
				ASSERT(i>=recSize()) ;
				lseek(_fd,wpos,SEEK_SET) ;
				if ( WRITE(_fd,r,i) != i )
					_errNo = DB_WRITE_ERR ;
				wpos += i ;
			}
		} else {
			wpos += len ;
		}

		rpos += len ;
	}

	if ( rpos == wpos )	/* not change */
		return _errNo == DB_SUCCESS ? DB_SUCCESS : DB_FAILURE ;

	_isUpdated |= UPD_HEADER ;
	_recCount = (wpos - _headSize) / recSize() ;

	if ( flush() == DB_FAILURE ) return DB_FAILURE ;
	chsize(_fd,wpos) ;
	return recCount() > 0 ? top() : DB_SUCCESS ;
}

