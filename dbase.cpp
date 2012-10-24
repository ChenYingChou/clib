/*
 * $Log$
 *
**/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include "dbase.h"
#include "dbase0.h"
#if defined(UNIX)
    #include "str.h"
#endif

/******************************************************************************/

int DBF::initial ( void *H )
{
	HEADER	&h = (HEADER&)(*(HEADER*)H) ;

	_headSize    = h.headSize ;
	_recCount    = h.recCount ;
	_recSize     = h.recSize ;
#ifdef	PROTECT
	_protectCode = h.protectCode ;
#endif

	_updYear     = h.date[0] + 1900;
	_updMonth    = h.date[1];
	_updDay      = h.date[2];
	if (_updYear < 1970) _updYear += 100;

	_fieldCount  = (_headSize >> 5) - 1 ;
	_recNo	     = -2 ;
	_isUpdated   = 0 ;
	_errNo	     = DB_SUCCESS ;

	_field	     = new Field[_fieldCount] ;
#if defined(__BORLANDC__)
	_data	     = new char[_recSize+1] ;	// need EOS for printf("%*.*s")
#else
	_data	     = new char[_recSize] ;
#endif

	if ( _field == NULL || _data == NULL ) {
		close() ;
		_errNo = DB_NO_MEMORY ;
		return DB_FAILURE ;
	}

	memset(_data,' ',_recSize) ;
#if defined(__BORLANDC__)
	// Borland C want to find EOS for printf("%*.*s ...")
	_data[_recSize] = '\0' ;
#endif
	memset(_field,0,_fieldCount*sizeof(Field)) ;

	FIELD	*F	;
	UINT	i	;

	F = new FIELD[_fieldCount] ;
	if ( F == NULL ) {
		close() ;
		_errNo = DB_NO_MEMORY ;
		return DB_FAILURE ;
	}

	lseek(_fd,sizeof(HEADER),SEEK_SET) ;
	i = _fieldCount * sizeof(FIELD) ;
	if ( READ(_fd,F,i) != i ) {
		delete[] F ;
		close() ;
		_errNo = DB_STRUC_ERR ;
		return DB_FAILURE ;
	}

	FIELD	*f = F ;
	UINT	offset = 1 ;
	for ( i = 0 ; i < _fieldCount ; i++, f++ ) {
		_field[i].offset  = offset ;
		strncpy(_field[i].name,f->name,sizeof(f->name)) ;
		_field[i].type	  = f->type ;
		_field[i].length  = f->length ;
		_field[i].decimal = f->decimal ;

		offset += f->length ;
	}
	delete[] F ;

	if ( offset != _recSize ) {
		close() ;
		_errNo = DB_STRUC_ERR ;
		return DB_FAILURE ;
	}

	if ( _cacheSize == 0 ) {
#if defined(__386__) || defined(UNIX)
		_cacheSize = 32 * 1024 ;
#elif defined(__LARGE__) || defined(__HUGE__) || defined(__COMPACT__)
		_cacheSize = 8 * 1024 ;
#else
		_cacheSize = 2 * 1024 ;
#endif
	}

	while ( _cacheSize >= 2 * _recSize ) {
		i = (_cacheSize / _recSize) * _recSize ;
		_cachePtr = new char[i] ;
		if ( _cachePtr ) {
			_cacheSize = i ;
			_cacheFirst = _cacheNext = 0 ;
			break ;
		}
		_cacheSize /= 2 ;
	}
	if ( _cachePtr == NULL ) _cacheSize = 0 ;

	return DB_SUCCESS ;
}

/******************************************************************************/

DBF::DBF ( const char *file, unsigned cacheSize )
{
	_fd	   = -1 ;
	_field	   = NULL ;
	_data	   = NULL ;
	_cachePtr  = NULL ;
	_cacheSize = cacheSize ;
	_isUpdated = 0 ;
	_errNo	   = DB_NOT_OPEN ;
	_doFlush   = 1;
	if ( file ) open(file) ;
}


int DBF::open ( const char *file, unsigned cacheSize )
{
	if ( _fd >= 0 ) close() ;
#if defined(UNIX)
	_fd = OPEN(makeFileExt(file,".dbf")) ;
#else
	_fd = OPEN(file) ;
#endif
	if ( _fd < 0 ) {
		_errNo = DB_OPEN_ERR ;
		return DB_FAILURE ;
	}

	HEADER	h	;
	if ( READ(_fd,&h,sizeof(h)) != sizeof(h) || (h.signature & 0x77) != DBF_SIGNATURE ||
	     h.headSize > MAX_HEAD_SIZE ) {
		close() ;
		_errNo = DB_STRUC_ERR ;
		return DB_FAILURE ;
	}

	if ( cacheSize ) _cacheSize = cacheSize ;
	return initial(&h) ;
}


int DBF::close ( )
{
	if ( _fd < 0 ) {
		_errNo = DB_NOT_OPEN ;
		return DB_FAILURE ;
	}

	flush() ;
	CLOSE(_fd) ;		_fd	  = -1 ;
	delete[] _field ;	_field	  = NULL ;
	delete[] _data ;	_data	  = NULL ;
	delete[] _cachePtr ;	_cachePtr = NULL ;

	return DB_SUCCESS ;
}

/******************************************************************************/

int DBF::Delete ( )
{
	if ( _fd < 0 ) {
		_errNo = DB_NOT_OPEN ;
		return DB_FAILURE ;
	}

	if ( _data[0] != '*' ) {
		modify() ;
		_data[0] = '*' ;
	}
	return DB_SUCCESS ;
}


int DBF::recall ( )
{
	if ( _fd < 0 ) {
		_errNo = DB_NOT_OPEN ;
		return DB_FAILURE ;
	}

	if ( _data[0] == '*' ) {
		modify() ;
		_data[0] = ' ' ;
	}
	return DB_SUCCESS ;
}


int DBF::flush_data ( )
{
	if ( _fd < 0 ) {
		_errNo = DB_NOT_OPEN ;
		return DB_FAILURE ;
	}

	if ( (_isUpdated & UPD_DATA) == 0 ) return DB_SUCCESS ;

	if ( _recNo == -1 ) _recNo = _recCount++ ;	// append

	if ( (unsigned long)_recNo >= (unsigned long)_recCount ) {
		_errNo = DB_RECORD_OUT_RANGE ;
		return DB_FAILURE ;
	}

#ifdef	PROTECT
	if ( _protectCode )
		encode(_data+1,_recSize-1,_protectCode) ;
#endif

	lseek(_fd,_headSize+_recNo*_recSize,SEEK_SET) ;
	_errNo = (WRITE(_fd,_data,_recSize)==_recSize ? DB_SUCCESS : DB_WRITE_ERR) ;

	if ( _recNo < _cacheNext && _cacheSize && _recNo >= _cacheFirst ) {
		memcpy(_cachePtr+(_recNo-_cacheFirst)*_recSize,_data,_recSize) ;
	}

#ifdef	PROTECT
	if ( _protectCode )
		decode(_data+1,_recSize-1,_protectCode) ;
#endif

	_isUpdated |= UPD_HEADER ;
	_isUpdated &= ~UPD_DATA ;

	return _errNo == DB_SUCCESS ? DB_SUCCESS : DB_FAILURE ;
}


int DBF::read ( long recNo )
{
	if ( flush_data() == DB_FAILURE ) return DB_FAILURE ;

	if ( (unsigned long)(--recNo) >= (unsigned long)_recCount ) {
		_errNo = DB_RECORD_OUT_RANGE ;
		return DB_FAILURE ;
	}

	_recNo = recNo ;
	if ( _cacheSize ) {
		if ( _recNo < _cacheFirst || _recNo >= _cacheNext ) {
			// not in cache, reading
			lseek(_fd,_headSize+_recNo*_recSize,SEEK_SET) ;
			int	n = READ(_fd,_cachePtr,_cacheSize) / _recSize ;
			_cacheFirst = _recNo ;
			_cacheNext  = _cacheFirst + n ;
			if ( n <= 0 ) {
				_errNo = DB_READ_ERR ;
				return DB_FAILURE ;
			}
		}
		memcpy(_data,_cachePtr+(_recNo-_cacheFirst)*_recSize,_recSize) ;
	} else {	// no cache
		lseek(_fd,_headSize+_recNo*_recSize,SEEK_SET) ;
		if ( READ(_fd,_data,_recSize) != _recSize ) {
			_errNo = DB_READ_ERR ;
			return DB_FAILURE ;
		}
	}

#ifdef	PROTECT
	if ( _protectCode )
		decode(_data+1,_recSize-1,_protectCode) ;
#endif
	return DB_SUCCESS ;
}


int DBF::append ( )
{
	if ( flush_data() == DB_FAILURE ) return DB_FAILURE ;

	_recNo = -1 ;
	modify() ;
	memset(_data,' ',_recSize) ;

	return DB_SUCCESS ;
}


int DBF::flush ( )
{
	if ( flush_data() == DB_FAILURE ) return DB_FAILURE ;
	if ( (_isUpdated & UPD_HEADER) == 0 ) return DB_SUCCESS ;

	time_t	T = time(NULL) ;
	tm	*t = localtime(&T) ;

	_updYear  = t->tm_year + 1900;
	_updMonth = t->tm_mon  + 1;
	_updDay   = t->tm_mday;

	HEADER	x	;
	memset(&x,0,sizeof(x)) ;
	x.signature	= DBF_SIGNATURE ;
	x.date[0]	= t->tm_year ;		// since 1900
	x.date[1]	= _updMonth;
	x.date[2]	= _updDay;
	x.recCount	= _recCount ;
	x.headSize	= _headSize ;
	x.recSize	= _recSize ;
#ifdef	PROTECT
	x.protectCode	= _protectCode ;
#endif

	lseek(_fd,0L,SEEK_SET) ;
	_errNo = (WRITE(_fd,&x,sizeof(x))==sizeof(x) ? DB_SUCCESS : DB_WRITE_ERR) ;
	if ( _doFlush ) FLUSH(_fd);

	_isUpdated &= ~ UPD_HEADER ;

	return _errNo == DB_SUCCESS ? DB_SUCCESS : DB_FAILURE ;
}
/******************************************************************************/

char *DBF::fieldName ( int nth )
{
	return nth >= fieldCount() ? NULL : _field[nth].name ;
}


int DBF::field ( const char *name )
{
	int	nth	;

	for ( nth = 0 ; nth < fieldCount() ; nth++ ) {
#if defined(GNUC)
		if ( strcasecmp(name,_field[nth].name) == 0 )
#else
		if ( stricmp(name,_field[nth].name) == 0 )
#endif
			return nth ;
	}
	_errNo = DB_FIELD_NOT_FOUND ;
	return -1 ;
}


char *DBF::fieldPtr ( const char *name )
{
	int	n = field(name) ;
	return n < 0 ? NULL : fieldPtr(n) ;
}


int DBF::fieldType ( const char *name )
{
	int	n = field(name) ;
	return n < 0 ? -1 : fieldType(n) ;
}


int DBF::fieldLength ( const char *name )
{
	int	n = field(name) ;
	return n < 0 ? -1 : fieldLength(n) ;
}


int DBF::fieldDecimal ( const char *name )
{
	int	n = field(name) ;
	return n < 0 ? -1 : fieldDecimal(n) ;
}


char *DBF::gets ( int nth, char *str, int len )
{
	int	l = fieldLength(nth) ;

	if ( l >= len ) l = len - 1 ;
	memcpy(str,fieldPtr(nth),l) ;
	str[l] = '\0' ;
	return str ;
}


char *DBF::gets ( const char *name, char *str, int len )
{
	int	nth = field(name) ;
	return nth < 0 ? NULL : gets(nth,str,len) ;
}


int DBF::puts ( int nth, const char *str, int len )
{
	char	*p = fieldPtr(nth) ;
	int	l = fieldLength(nth) ;
	int	isNum = fieldType(nth) == 'N';

	if ( len == 0 ) len = strlen(str) ;
	if ( isNum ) {
		while (len > 0 && (*str == '0' || *str == ' ')) {
			str++; len--;
		}
	}

	if ( len >= l ) {
		/* Allow the first digit overflow represented by A-Z */
		if ( len == l+1 && isNum && memcmp(str,"35",2) <= 0 ) {
			*p++ = (str[0]-'1') * 10 + 'A' - '0' + str[1];
			l--;
			str += 2;
		}
		len = l ;
	} else {
		memset(p,' ',l) ;
		if ( isNum ) p += l - len ; // 'N': right adjust
	}

	memcpy(p,str,len) ;
	modify() ;
	return 0 ;
}


int DBF::puts ( const char *name, const char *str, int len )
{
	int	nth = field(name) ;
	return nth < 0 ? -1 : puts(nth,str,len) ;
}


long DBF::getLong ( int nth )
{
	char	*p = fieldPtr(nth) ;
	int	l = fieldLength(nth) ;
	long	x = 0;
	int	isMinus = 0;

	// Skip leading spaces
	while ( l > 0 && *p <= ' ') {
		p++; l--;
	}

	/* Allow the first digit overflow represented by A-Z */
	while ( l > 0 && isalpha(*p) ) {
	    x = 10 * x + toupper(*p) - 'A' + 10;
	    p++; l--;
	}

	while ( l > 0 ) {
		if ( isdigit(*p) ) {
			x = 10 * x + *p - '0' ;
		} else if ( *p == '-' ) {
			isMinus = !isMinus;
		}
		p++ ; l-- ;
	}
	return isMinus ? -x : x;
}


long DBF::getLong ( const char *name )
{
	int	nth = field(name) ;
	return nth < 0 ? -1 : getLong(nth) ;
}


int DBF::putLong ( int nth, long val )
{
	char	*p = fieldPtr(nth) ;
	int	ll = fieldLength(nth) ;
	int	sign = 0 ;

	modify() ;

	if ( val < 0 ) {
		sign = 1 ;
		val = -val ;
	}

	int l = ll ;
	p += l ;
	do {
		*--p = '0' + (int)(val%10) ;
		val /= 10 ;
		l-- ;
	} while ( l > 0 && val != 0 ) ;

	if ( l == 0 ) {
	    	if ( !sign && (val >= 1  && val <= 3) ) {
			/* Allow the first digit overflow represented by A-Z */
			*p += 'A' - '0' + (val-1) * 10;
			if ( isalpha(*p) ) val = 0;
		}

		if ( sign || val != 0 ) {	// overflow
			memset(p,'*',ll) ;
			_errNo = DB_FIELD_OVERFLOW ;
			return -1 ;
		}
	}

	if ( sign ) {
		*--p = '-' ;
		l-- ;
	}

	while ( l > 0 ) {
		*--p = ' ' ;
		l-- ;
	}

	return 0 ;
}


int DBF::putLong ( const char *name, long val )
{
	int	nth = field(name) ;
	return nth < 0 ? -1 : putLong(nth,val) ;
}

