/*
 * $Log: TWIN1.CPP $
 * Revision 1.0  1996-02-06 23:29:13-0800  YCCHEN
 * Initial revision
 *

void twin::scroll_block_down( int row1, int col1, int row2, int col2,
			      int numRow )				;

*/

#include	<string.h>
#include	"twin.h"

/*****************************************************************************/

#if defined(__WATCOMC__)

	void Bios_scroll_down ( Byte n, Byte attr, Byte r1, Byte c1, Byte r2, Byte c2 ) ;
	#pragma aux Bios_scroll_down = \
			"mov    ah,7"           \
			"push   bp"             \
			"int    10h"            \
			"pop    bp"             \
			parm [al] [bh] [ch] [cl] [dh] [dl]	\
			modify [ah]		;

	void memsetW ( Wfptr p, int ch, int len ) ;
	#if defined(__386__)
		#pragma aux memsetW = \
				"cld"                   \
				"rep    stosw"          \
				parm [edi] [eax] [ecx]	\
				modify [ecx edi]	;
	#else	/* 16 bits mode */
		#pragma aux memsetW = \
				"cld"                   \
				"rep    stosw"          \
				parm [es di] [ax] [cx]	\
				modify [cx di]		;
	#endif

#elif defined(__DJGPP__)

	#include <dpmi.h>
	#include <go32.h>
	#include <sys/nearptr.h>

	#define _fmemmove(dest,src,cnt) 	memmove(dest,src,cnt)

	static void Bios_scroll_down ( Byte n, Byte attr, Byte r1, Byte c1,
				       Byte r2, Byte c2 )
	{
		__dpmi_regs r ;

		r.h.ah = 7 ;
		r.h.al = n ;
		r.h.bh = attr ;
		r.h.ch = r1 ;
		r.h.cl = c1 ;
		r.h.dh = r2 ;
		r.h.dl = c2 ;
		__dpmi_int(0x10,&r) ;
	}


	static __inline__ void memsetW ( Wfptr p, int ch, int len )
	{
		asm("   cld
			rep
			stosw	"
		    :
		    : "D" (p), "a" (ch), "c" (len)
		    : "ecx", "edi" ) ;
	}

#else

	static void near Bios_scroll_down ( Byte n, Byte attr, Byte r1, Byte c1,
					    Byte r2, Byte c2 )
	{
		asm {
			mov	al,n
			mov	bh,attr
			mov	ch,r1
			mov	cl,c1
			mov	dh,r2
			mov	dl,c2
			mov	ah,7
			push	bp
			int	10h
			pop	bp
		}
	}

	void memsetW ( Wfptr p, int wData, int len ) ;

#endif

/*****************************************************************************/

static void mem_scroll_down ( Wfptr p, int W, int w, int h, int n, int ch )
{
	int	cnt	;

	if ( n > 0 && (cnt=h-n) > 0 ) {
		if ( W == w ) { /* continuous blocks */
			_fmemmove(p+n*W,p,2*cnt*W) ;
		} else {	/* move row by row: bottom-up */
			Wfptr	q = p + h * W	;
			p += cnt * W ;
			while ( cnt > 0 ) {
				p -= W ;
				q -= W ;
				cnt-- ;
				_fmemmove(q,p,2*w) ;
			}
		}
	} else {	/* clear all */
		n = h ;
	}

	if ( W == w ) {
		memsetW(p,ch,n*W) ;
	} else {
		while ( n > 0 ) {
			memsetW(p,ch,w) ;
			p += W ;
			n-- ;
		}
	}
}


void twin::scroll_block_down ( int row1, int col1, int row2, int col2,
			       int numRow )
{
	int	r1, r2, c1, c2	;

	r1 = row1 + vR0_ ;
	if ( r1 < vR1_ )
		r1 = vR1_ ;
	else if ( r1 > vR2_ )
		r1 = vR2_ ;

	r2 = row2 + vR0_ ;
	if ( r2 < vR1_ )
		r2 = vR1_ ;
	else if ( r2 > vR2_ )
		r2 = vR2_ ;

	c1 = col1 + vC0_ ;
	if ( c1 < vC1_ )
		c1 = vC1_ ;
	else if ( c1 > vC2_ )
		c1 = vC2_ ;

	c2 = col2 + vC0_ ;
	if ( c2 < vC1_ )
		c2 = vC1_ ;
	else if ( c2 > vC2_ )
		c2 = vC2_ ;


	if ( toBuffer() && pBuffer_ != 0 ) {
		int	W = _C2_ - _C1_ + 1 ;
		Wfptr	p = pBuffer_ + ((r1-_R1_)*W+c1-_C1_) ;
		mem_scroll_down(p,W,c2-c1+1,r2-r1+1,
			      numRow,' '+((int)attr_<<8)) ;
	}

	if ( toVideo() ) {
		Wfptr	p = videoPtr + (r1*videoWidth+c1) ;
#if defined(__DJGPP__)
		DJNearptr_enable() ;
		p = (Wfptr)((Bfptr)p + __djgpp_conventional_base) ;
#endif
		mem_scroll_down(p,videoWidth,c2-c1+1,r2-r1+1,
			      numRow,' '+((int)attr_<<8)) ;
#if defined(__DJGPP__)
		DJNearptr_disable() ;
#endif
	} else if ( toBios() ) {
		Bios_scroll_down(numRow,attr_,r1,c1,r2,c2) ;
	}
}

/*****************************************************************************/

