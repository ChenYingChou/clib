/*
 * $Log$
 *
**/

#if !defined(_DBASE0_H_)
#define _DBASE0_H_

#include <fcntl.h>
#include <io.h>

#if defined(__DJGPP__)
#include <unistd.h>
#endif

#if !defined(UINT)
#define UINT			unsigned
#endif

#if defined(MSDOS) || defined(__MSDOS__)
	#include <dos.h>
	#include <share.h>

	#define CLOSE(fd)		_dos_close(fd)
	#define FLUSH(fd)		_dos_commit(fd)

	static inline int OPEN ( const char *file )
	{
		int	fd	;
		if ( _dos_open(file,O_RDWR|SH_DENYNO,&fd) != 0 ) return -1 ;
		return fd ;
	}

	static inline int CREAT ( const char *file )
	{
		int	fd	;
		if ( _dos_creat(file,_A_NORMAL,&fd) != 0 ) return -1 ;
		return fd ;
	}

	static inline unsigned READ ( int fd, void *buf, unsigned sz )
	{
		UINT	readLen ;
		if ( _dos_read(fd,buf,sz,&readLen) != 0 ) return 0 ;
		return readLen ;
	}

	static inline unsigned WRITE ( int fd, void *buf, unsigned sz )
	{
		UINT	writeLen ;
		if ( _dos_write(fd,buf,sz,&writeLen) != 0 ) return 0 ;
		return writeLen ;
	}
#else
	#include < sys/stat.h>

	#define OPEN(file)		::open(file,O_RDWR)
	#define CREAT(file)		::creat(file,S_IRUSR|S_IWUSR)
	#define CLOSE(fd)		::close(fd)
	#define READ(fd,buf,sz) 	::read(fd,buf,sz)
	#define WRITE(fd,buf,sz)	::write(fd,buf,sz)
	#define FLUSH(fd)		fsync(fd)
#endif

#define DBF_SIGNATURE		0x03
#define MAX_FIELDS		512
#define MAX_HEAD_SIZE		(sizeof(HEADER)+MAX_FIELDS*sizeof(FIELD))

#pragma pack(1) ;

struct	FIELD	{
	char	name[10]	;
	char	filler1 	;	/* NUL				*/
	char	type		;	/* 'C', 'N', 'L'                */
	char	filler2[4]	;
	Byte	length		;
	Byte	decimal 	;	/* decimal number		*/
	char	filler3[14]	;
} ;

struct	HEADER	{
	Byte	signature	;	/* 0x03 			*/
	Byte	date[3] 	;
	long	recCount	;
	Word	headSize	;
	Word	recSize 	;
	char	filler1[18]	;
	Word	protectCode	;	/* protect flag 		*/
} ;

#pragma pack() ;

#endif
