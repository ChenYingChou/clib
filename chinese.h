/*
 * $Log: CHINESE.H $
 * Revision 1.0  1996-02-06 23:29:13-0800  YCCHEN
 * Initial revision
 *
**/

#if !defined(_CHINESE_H_)
#define _CHINESE_H_

#ifdef __cplusplus
extern "C" {
#endif

extern	int  is_chinese( unsigned char c )				;
extern	int  is_chinese2( const char str[], int nth )			;

#ifdef __cplusplus
};
#endif

#endif	/* _CHINESE_H_ */

