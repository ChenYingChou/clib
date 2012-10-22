// $log: doall.h $
//

#if !defined(_DOALL_H_)
#define _DOALL_H_

#include	<dos.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*doallFunc)( char *fullpath, struct find_t *t ) ;

int do_all ( const char *filespec, doallFunc func, unsigned attrib ) ;

#ifdef __cplusplus
};
#endif

#endif	// _DOALL_H_
