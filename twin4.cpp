/*
 * $Log: TWIN4.CPP $
 * Revision 1.0  1996-02-06 23:29:13-0800  YCCHEN
 * Initial revision
 *

int twin::getca( int row, int col )					;

*/

#include	"twin.h"

/*****************************************************************************/

#if defined(__WATCOMC__)

	void Bios_locate ( Byte page, Byte row, Byte col ) ;
	#pragma aux Bios_locate = \
			"mov    ah,2"           \
			"push   bp"             \
			"int    10h"            \
			"pop    bp"             \
			parm [bh] [dh] [dl]	\
			modify [ah]		;

	Word Bios_getca ( Byte page ) ;
	#pragma aux Bios_getca = \
			"mov    ah,8"           \
			"push   bp"             \
			"int    10h"            \
			"pop    bp"             \
			parm [bh]		\
			value [ax]		;

#elif defined(__DJGPP__)

	#include <dpmi.h>
	#include <go32.h>
	#include <sys/farptr.h>

	static void Bios_locate ( Byte page, Byte row, Byte col )
	{
		__dpmi_regs r ;

		r.h.ah = 2 ;
		r.h.bh = page ;
		r.h.dh = row ;
		r.h.dl = col ;
		__dpmi_int(0x10,&r) ;
	}


	static int Bios_getca ( Byte page )
	{
		__dpmi_regs r ;

		r.h.ah = 8 ;
		r.h.bh = page ;
		__dpmi_int(0x10,&r) ;
		return r.x.ax ;
	}

#else	/* Borland C */

	static void near Bios_locate ( Byte page, Byte row, Byte col )
	{
		asm {
			mov	bh,page
			mov	dh,row
			mov	dl,col
			mov	ah,2
			push	bp
			int	10h
			pop	bp
		}
	}


	static int near Bios_getca ( Byte page )
	{
		asm {
			mov	bh,page
			mov	ah,8
			push	bp
			int	10h
			pop	bp
		}
		return _AX ;
	}

#endif

/*****************************************************************************/

int twin::getca ( int row, int col )
{
	int	r, c	;

	r = (row==0 ? row_+vR1_ : row+vR0_) ;
	c = (col==0 ? col_+vC1_ : col+vC0_) ;

	if ( toVideo() )
#if defined(__DJGPP__)
		return _farpeekw(_dos_ds,(long)(&videoPtr[r*videoWidth+c])) ;
#else
		return videoPtr[r*videoWidth+c] ;
#endif

	if ( toBuffer() && pBuffer_ != 0 )
		return pBuffer_[(r-_R1_)*(_C2_-_C1_+1)+(c-_C1_)] ;

	int	ca	;
	int	notCurr = (r!=row_ || c!=col_)	;

	if ( notCurr ) Bios_locate(page_,r,c) ;
	ca = Bios_getca(page_) ;
	if ( notCurr ) Bios_locate(page_,row_,col_) ;

	return ca ;
}

/*****************************************************************************/

