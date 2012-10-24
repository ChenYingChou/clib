/*
 * $Log: TYPEDEF.H $
 *
**/

#ifndef _TYPEDEF_
#define _TYPEDEF_	1

#include <sys/types.h>

#undef	UNIX
#undef	LLONG
#undef	GNUC
#if defined(linux) || defined(__APPLE__) || defined(__FreeBSD__)
	// Linux GNU C++
	#include <stdint.h>
	#define	UNIX		1
	#define	LLONG		1
	#define	GNUC		1
    #if defined(__APPLE__) || defined(__FreeBSD__)
	#define	FreeBSD		1
    #endif
#elif defined(_WIN32) || defined(__DJGPP__)
	// WIN/DOS GNU C++
	#include <stdint.h>
	#define	LLONG		1
	#define	GNUC		1
#elif defined(__386__)
	// WATCOM C++/386
    #ifndef __int8_t_defined
	# define __int8_t_defined
	typedef signed char		int8_t;
	typedef short int		int16_t;
	typedef int			int32_t;
    #endif

	/* Unsigned.  */
	typedef unsigned char		uint8_t;
	typedef unsigned short int	uint16_t;
    #ifndef __uint32_t_defined
	typedef unsigned int		uint32_t;
	# define __uint32_t_defined
    #endif
#elif defined(__BORLANDC__) || defined(__WATCOMC__)
    #ifndef __int8_t_defined
	# define __int8_t_defined
	typedef signed char		int8_t;
	typedef int			int16_t;
	typedef long			int32_t;
    #endif

	/* Unsigned.  */
	typedef unsigned char		uint8_t;
	typedef unsigned int		uint16_t;
    #ifndef __uint32_t_defined
	typedef unsigned long		uint32_t;
	# define __uint32_t_defined
    #endif
#endif

#if !defined(_Byte_)
#define _Byte_		1

	typedef unsigned int	Bit	;
	typedef unsigned char	Byte	;
	typedef unsigned short	Word	;
	typedef int32_t		Int4	;
	typedef uint32_t	Dword	;
	typedef Byte *		Bptr	;
	typedef Word *		Wptr	;
    #if defined(__386__) || defined(LLONG)
	#define FAR
	#define NEAR
    #else
	#define FAR		far
	#define NEAR		near
    #endif
	typedef Byte FAR *	Bfptr	;
	typedef Word FAR *	Wfptr	;

	typedef	unsigned char	BCD	;

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

#endif	/* _TYPEDEF_ */
