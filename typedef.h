/*
 * $Log: TYPEDEF.H $
 * Revision 1.0  1996-02-06 23:29:14-0800  YCCHEN
 * Initial revision
 *
**/

#ifndef _TYPEDEF_H_
#define _TYPEDEF_H_	1

#if !defined(_Byte_)
#define _Byte_		1

	typedef unsigned int	Bit	;
	typedef unsigned char	Byte	;
	typedef unsigned short	Word	;
	typedef unsigned long	Dword	;
	typedef Byte *		Bptr	;
	typedef Word *		Wptr	;
    #if defined(__386__)
	#define FAR
	#define NEAR
    #else
	#define FAR		far
	#define NEAR		near
    #endif
	typedef Byte FAR *	Bfptr	;
	typedef Word FAR *	Wfptr	;
#endif

#ifndef GLOBAL
#define GLOBAL
#endif

#ifndef TRUE
#define TRUE		1
#endif

#ifndef FALSE
#define FALSE		0
#endif

#ifndef YES
#define YES		1
#endif

#ifndef NO
#define NO		0
#endif

#ifndef ON
#define ON		1
#endif

#ifndef OFF
#define OFF		0
#endif

#ifndef NIL
#define NIL		0
#endif

#if defined(__WATCOMC__)
    #define NOT_USE(argv)		/* argv */
#else
    #define NOT_USE(argv)		if ( argv ) ;
#endif

#endif
