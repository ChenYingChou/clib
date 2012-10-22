/*
 * $Log$
 *
**/

#if !defined(_DBASE_H_)
#define _DBASE_H_

#include	<stdio.h>
#include	"typedef.h"

enum	{
	  DB_FAILURE=-1, DB_SUCCESS=0, DB_NOT_OPEN, DB_STRUC_ERR, DB_NO_MEMORY,
	  DB_READ_ERR, DB_WRITE_ERR, DB_RECORD_OUT_RANGE,
	  DB_FIELD_NOT_FOUND, DB_FIELD_TYPE_MISSING, DB_FIELD_OVERFLOW,
	  DB_OPEN_ERR, DB_CREATE_ERR
} ;

/******************************************************************************/
#pragma pack(1)

struct	DBfield {
	char	name[11]	;
	char	type		;	/* 'C', 'N', 'L'                */
	Byte	length		;
	Byte	decimal 	;	/* decimal number		*/
} ;

#pragma pack()
/******************************************************************************/

class	DBF	{
    private:
	enum	{ UPD_HEADER=0x01, UPD_DATA=0x02 } ;

	struct	Field	{		// internal use
		short	offset		;
		char	name[11]	;
		char	type		;	/* 'C', 'N', 'L'                */
		Byte	length		;
		Byte	decimal 	;	/* decimal number		*/
	} ;

	int	_fd		;
	Field	*_field 	;
	char	*_data		;
	long	_recCount	;
	long	_recNo		;
	int	_headSize	;
	int	_recSize	;
	int	_fieldCount	;
	int	_isUpdated	;
	int	_errNo		;
	int	_doFlush	;
	int	_updYear	;
	int	_updMonth	;
	int	_updDay		;
#ifdef	PROTECT
	int	_protectCode	;
#endif

	/* records cache */
	char	*_cachePtr	;
	unsigned _cacheSize	;
	long	_cacheFirst	;
	long	_cacheNext	;

	int initial ( void *H )						;
	int flush_data ( )						;

    public:
	~DBF ( ) { close() ; }						;
	DBF ( const char *file=0, unsigned cacheSize=0 )		;

	int  create ( const char *file, DBfield field[], int numfield ) ;
	int  open ( const char *file, unsigned cacheSize=0 )		;
	int  close ( )							;
	int  read ( long recNo )					;
	int  Delete ( ) /* Too bad, can't named "delete" */             ;
	int  append ( ) 						;
	int  flush ( )							;
	int  getFlush ( ) { return _doFlush				;}
	void setFlush ( int bFlush ) { _doFlush = bFlush		;}
	int  recall ( ) 						;
	int  pack ( )							;
	int  insert ( long recNum )					;

	char *fieldName ( int nth )					;
	int  field ( const char *name ) 				;

	char *fieldPtr ( const char *name )				;
	int  fieldType ( const char *name )				;
	int  fieldLength ( const char *name )				;
	int  fieldDecimal ( const char *name )				;

	char *fieldPtr ( int nth )    { return _data+_field[nth].offset ;}
	int  fieldType ( int nth )    { return _field[nth].type 	;}
	int  fieldLength ( int nth )  { return _field[nth].length	;}
	int  fieldDecimal ( int nth ) { return _field[nth].decimal	;}

	char *gets ( int nth, char *str, int len )			;
	char *gets ( const char *name, char *str, int len )		;
	int  puts ( int nth, const char *str, int len=0 )		;
	int  puts ( const char *name, const char *str, int len=0 )	;
	long getLong ( int nth )					;
	long getLong ( const char *name )				;
	int  putLong ( int nth, long val )				;
	int  putLong ( const char *name, long val )			;
	double getNumber ( int nth )					;
	double getNumber ( const char *name )				;
	int  putNumber ( int nth, double val )				;
	int  putNumber ( const char *name, double val ) 		;

	char *record ( )	{ return _data				;}
	int  go ( long recNo )	{ return read(recNo)			;}
	int  top ( )		{ return read(1)			;}
	int  bottom ( ) 	{ return read(_recCount)		;}
	int  skip ( long num=1 ){ return read(recNo()+num)		;}
	int  write ( )		{ return flush_data()			;}
	long recCount ( )	{ return _recCount			;}
	long recNo ( )		{ return _recNo+1			;}
	int  headSize ( )	{ return _headSize			;}
	int  recSize ( )	{ return _recSize			;}
	int  fieldCount ( )	{ return _fieldCount			;}
	int  isOpen ( ) 	{ return _fd >= 0			;}
	int  isDeleted ( )	{ return _data[0] == '*'                ;}
	int  errNo ( )		{ return _errNo 			;}
	int  eof ( )		{ return _errNo == DB_RECORD_OUT_RANGE	;}
	void modify ( ) 	{ _isUpdated |= UPD_DATA		;}
	int  updatedYear ( )	{ return _updYear			;}
	int  updatedMonth ( )	{ return _updMonth			;}
	int  updatedDay ( )	{ return _updDay			;}
} ;

#endif
