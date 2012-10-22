/*
 * $Log: DEBUG.H $
 * Revision 1.1  1996-07-15 01:04:51-0700  ycchen
 * Initial revision
 *
**/

#if !defined(_DEBUG_H_)
#define _DEBUG_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifdef	DEBUG
	void	_AssertMsg(const char *sFile, int nLine) ;
	#define ASSERTFILE(str) 	\
		static	char	_strAssertFile[] = str ;
	#define ASSERT(f)		\
		if ( f )		\
			(void)0 ;	\
		else			\
			_AssertMsg(_strAssertFile,__LINE__)
#else
	#define ASSERTFILE(str)
	#define ASSERT(f)		(void)0
#endif

#ifdef __cplusplus
};
#endif

#endif	/* _DEBUG_H_ */

