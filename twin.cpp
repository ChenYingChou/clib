/*
 * $Log: TWIN.CPP $
 * Revision 1.0  1996-02-06 23:29:12-0800  YCCHEN
 * Initial revision
 *
-------------------------------------------------------------------------------
int	twin::videoWidth       ;
int	twin::videoHigh        ;
Byte	twin::videoMode        ;
Byte	twin::isColor	       ;
Wfptr	twin::videoPtr	       ;
twin	*twin::current	       ;
twin	stdscr		       ;
int	ChineseSW	       ;
int	GMode		       ;
int	directvideo	       ;
-------------------------------------------------------------------------------
void  twin_outs( twin &x, const char *s, int dir )			;
twin::~twin( )								;
twin::twin( int row1, int col1, int row2, int col2 )			;
void  twin::set_toBuffer( Bit onoff )					;
twin *twin::use( )							;
void  twin::set_view( int row1, int col1, int row2, int col2 )		;
void  twin::subview( int row1, int col1, int row2, int col2 )		;
void  twin::set_cursor( int type, int high, int low )			;
void  twin::locate( int row, int col )					;
void  twin::scroll_block_up( int row1, int col1, int row2, int col2,
			    int numRow )				;
void  twin::outs( const void *str )					;
int   twin::putch( int ch )						;
void  twin::outc( int ch, int len )					;
-------------------------------------------------------------------------------
*/

#if defined(__WATCOMC__)
	#pragma initialize library
	#include <i86.h>
#elif defined(__DJGPP__)
	#include <dpmi.h>
	#include <go32.h>
	#include <pc.h>
	#include <sys/farptr.h>
	#include <sys/nearptr.h>
	#define _fmemmove(dest,src,cnt) 	memmove(dest,src,cnt)
#else	/* Borland C */
	#include <dos.h>
#endif

#include	<stdlib.h>
#include	<stdio.h>
#include	<stdarg.h>
#include	<string.h>

#define 	_TWIN_
#include	"twin.h"

#define MAX_ROW 		25
#define MAX_COL 		80
#define VIDEO_ATTR_NORMAL	0x07

#define TO_BUFFER		0
#define TO_VIDEO		1
#define TO_BIOS 		2
#define isBuffer(dir)		(dir==TO_BUFFER)
#define isVideo(dir)		(dir==TO_VIDEO)
#define isBios(dir)		(dir==TO_BIOS)
#define isVisible(ctype)	(ctype!=1)

#if defined(__386__)
	#define BIOS_SEG	0
	#define VIDEO_MODE_OFS	0x00449
	#define VIDEO_WIDTH_OFS 0x0044a
	#define VIDEO_HIGH_OFS	0x00484 	 /* last row number */
	#define COLOR_SEG	0
	#define COLOR_OFS	0xb8000
	#define MONO_SEG	0
	#define MONO_OFS	0xb0000
    #if defined(__DJGPP__)
	#define MK_FP(seg,off)	((seg<<4)+off)
    #endif
#else
	#define BIOS_SEG	0x0040
	#define VIDEO_MODE_OFS	0x0049
	#define VIDEO_WIDTH_OFS 0x004a
	#define VIDEO_HIGH_OFS	0x0084		/* last row number */
	#define COLOR_SEG	0xb800
	#define COLOR_OFS	0
	#define MONO_SEG	0xb000
	#define MONO_OFS	0
#endif

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

	Word Bios_get_locate ( Byte page ) ;
	#pragma aux Bios_get_locate = \
			"mov    ah,3"           \
			"push   bp"             \
			"int    10h"            \
			"pop    bp"             \
			parm [bh]		\
			value [dx]		\
			modify [ah cx]		;

	void Bios_set_cursor ( Byte type, Byte high, Byte low ) ;
	#pragma aux Bios_set_cursor = \
			"ror    al,1"           \
			"ror    al,1"           \
			"ror    al,1"           \
			"or     ch,al"          \
			"xor    bh,bh"          \
			"mov    ah,1"           \
			"push   bp"             \
			"int    10h"            \
			"pop    bp"             \
			parm [al] [ch] [cl]	\
			modify [ax bh ch]	;

	Word Bios_get_cursor ( void ) ;
	#pragma aux Bios_get_cursor = \
			"xor    bh,bh"          \
			"mov    ah,3"           \
			"push   bp"             \
			"int    10h"            \
			"pop    bp"             \
			value [cx]		\
			modify [ax bh dx]	;

	void Bios_scroll_up ( Byte n, Byte attr, Byte r1, Byte c1, Byte r2, Byte c2 ) ;
	#pragma aux Bios_scroll_up = \
			"mov    ah,6"           \
			"push   bp"             \
			"int    10h"            \
			"pop    bp"             \
			parm [al] [bh] [ch] [cl] [dh] [dl]	\
			modify [ah]		;

	void Bios_putc ( Byte c ) ;
	#pragma aux Bios_putc = \
			"mov    ah,14"          \
			"xor    bh,bh"          \
			"push   bp"             \
			"int    10h"            \
			"pop    bp"             \
			parm [al]		\
			modify [ah bh]		;

	void Bios_outc ( Byte c, int len, Byte page, Byte attr ) ;
	#pragma aux Bios_outc = \
			"mov    ah,9"           \
			"push   bp"             \
			"int    10h"            \
			"pop    bp"             \
			parm [al] [cx] [bh] [bl]\
			modify [ah]		;

	void memsetW ( Wfptr p, int ch, int len ) ;
	#if defined(__386__)
	#pragma aux memsetW = \
			"rep    stosw"          \
			parm [edi] [eax] [ecx]	\
			modify [ecx edi]	;
	#else	/* 16 bits mode */
	#pragma aux memsetW = \
			"rep    stosw"          \
			parm [es di] [ax] [cx]	\
			modify [cx di]		;
	#endif

#elif defined(__DJGPP__)

	static void Bios_locate ( Byte page, Byte row, Byte col )
	{
		__dpmi_regs r ;

		r.h.ah = 2 ;
		r.h.bh = page ;
		r.h.dh = row ;
		r.h.dl = col ;
		__dpmi_int(0x10,&r) ;
	}


	static Word Bios_get_locate ( Byte page )
	{
		__dpmi_regs r ;

		r.h.ah = 3 ;
		r.h.bh = page ;
		__dpmi_int(0x10,&r) ;
		return r.x.dx ;
	}


	static void Bios_set_cursor ( Byte type, Byte high, Byte low )
	{
		__dpmi_regs r ;

		r.h.ah = 1 ;
		r.h.bh = 0 ;
		r.h.ch = (type<<5) | high ;
		r.h.cl = low ;
		__dpmi_int(0x10,&r) ;
	}


	static Word Bios_get_cursor ( void )
	{
		__dpmi_regs r ;

		r.h.ah = 3 ;
		r.h.bh = 0 ;
		__dpmi_int(0x10,&r) ;
		return r.x.cx ;
	}


	static void Bios_scroll_up ( Byte n, Byte attr, Byte r1, Byte c1,
				     Byte r2, Byte c2 )
	{
		__dpmi_regs r ;

		r.h.ah = 6 ;
		r.h.al = n ;
		r.h.bh = attr ;
		r.h.ch = r1 ;
		r.h.cl = c1 ;
		r.h.dh = r2 ;
		r.h.dl = c2 ;
		__dpmi_int(0x10,&r) ;
	}


	static void Bios_putc ( Byte c )
	{
		__dpmi_regs r ;

		r.h.ah = 14 ;
		r.h.al = c ;
		r.x.bx = 0 ;
		__dpmi_int(0x10,&r) ;
	}


	static void Bios_outc ( Byte c, int len, Byte page, Byte attr )
	{
		__dpmi_regs r ;

		r.h.ah = 9 ;
		r.h.al = c ;
		r.h.bh = page ;
		r.h.bl = attr ;
		r.x.cx = len ;
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


	static Word near Bios_get_locate ( Byte page )
	{
		asm {
			mov	bh,page
			mov	ah,3
			push	bp
			int	10h
			pop	bp
		}
		return _DX ;
	}


	static void near Bios_set_cursor ( Byte type, Byte high, Byte low )
	{
		asm {
			mov	al,type
			mov	ch,high
			mov	cl,low
			ror	al,1
			ror	al,1
			ror	al,1
			or	ch,al
			mov	ah,1
			xor	bh,bh
			push	bp
			int	10h
			pop	bp
		}
	}


	static Word near Bios_get_cursor ( void )
	{
		asm {
			xor	bh,bh
			mov	ah,3
			push	bp
			int	10h
			pop	bp
		}
		return _CX ;
	}


	static void near Bios_scroll_up ( Byte nRow, Byte attr, Byte r1, Byte c1,
					  Byte r2, Byte c2 )
	{
		asm {
			mov	al,nRow
			mov	bh,attr
			mov	ch,r1
			mov	cl,c1
			mov	dh,r2
			mov	dl,c2
			mov	ah,6
			push	bp
			int	10h
			pop	bp
		}
	}


	static void near Bios_putc ( Byte c )
	{
		asm {
			mov	al,c
			mov	ah,14
			xor	bh,bh
			push	bp
			int	10h
			pop	bp
		}
	}


	static void near Bios_outc ( Byte c, int len, Byte page, Byte attr )
	{
		asm {
			mov	al,c
			mov	cx,len
			mov	bh,page
			mov	bl,attr
			mov	ah,9
			push	bp
			int	10h
			pop	bp
		}
	}


	void memsetW ( Wfptr p, int wData, int len )
	{
		asm {
			push	es
			les	di,dword ptr p
			mov	ax,wData
			mov	cx,len
			cld
			rep	stosw
			pop	es
		}
	}

#endif

/*****************************************************************************/

#if defined(__DJGPP__)

int	twin::videoWidth	;
int	twin::videoHigh 	;
Byte	twin::videoMode 	;
Byte	twin::isColor		;
Wfptr	twin::videoPtr		;

#else

int	twin::videoWidth = *(Bfptr)MK_FP(BIOS_SEG,VIDEO_WIDTH_OFS)	;
#if defined(VIDEO_HIGH_OFS)
int	twin::videoHigh  = *(Bfptr)MK_FP(BIOS_SEG,VIDEO_HIGH_OFS) + 1	;
#else
int	twin::videoHigh  = MAX_ROW					;
#endif

Byte	twin::videoMode  = *(Bfptr)MK_FP(BIOS_SEG,VIDEO_MODE_OFS)	;
Byte	twin::isColor	 = (twin::videoMode != 0x07)			;
Wfptr	twin::videoPtr	 = (Wfptr)(twin::isColor ? MK_FP(COLOR_SEG,COLOR_OFS)
						 : MK_FP(MONO_SEG,MONO_OFS)  ) ;

#endif

twin	stdscr								;
twin	*twin::current	 = &stdscr					;
int	ChineseSW	 = 1						;
int	GMode		 = gConfirm_|gKeepIns_|gDoSpecial_|gBinActive_	;

#if defined(__WATCOMC__) || defined(__DJGPP__)
int	directvideo	 = 1						;
#endif

/*****************************************************************************/

static void NEAR mem_scroll_up ( Wfptr p, int W, int w, int h, int n, int ch )
{
	int	cnt	;

	if ( n > 0 && (cnt=h-n) > 0 ) {
		Wfptr	q = p + n * W	;
		if ( W == w ) { /* continuous blocks */
			cnt *= W ;
			_fmemmove(p,q,2*cnt) ;
			p += cnt ;
		} else {	/* move row by row: top-down */
			while ( cnt > 0 ) {
				_fmemmove(p,q,2*w) ;
				p += W ;
				q += W ;
				cnt-- ;
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


void twin_outs ( twin &x, const char *s, int dir )
{
	Wfptr	p0, p		;
	int	W, w, h 	;
	int	Blank		;
	int	i, row, col	;
	Byte	c, isC1 	;

	if ( !isBios(dir) ) {
		if ( isBuffer(dir) ) {	/* toBuffer */
			if ( x.toBuffer() == 0 ) return ;
			W = x._C2_ - x._C1_ + 1 ;
			p0 = x.pBuffer_ ;
		} else {		/* toVideo */
			W = x.videoWidth ;
			p0 = x.videoPtr + x.vR1_ * W + x.vC1_ ;
#if defined(__DJGPP__)
			DJNearptr_enable() ;
			p0 = (Wfptr)((Bfptr)p0 + __djgpp_conventional_base) ;
#endif
		}
		Blank = ' ' + ((int)x.attr_ << 8) ;
	}

	w = x.vC2_ - x.vC1_ + 1 ;
	h = x.vR2_ - x.vR1_ + 1 ;

	row = x.row_ ;
	col = x.col_ ;

	if ( isBios(dir) ) {
		Bios_set_cursor(1,0,0) ;	/* cursor off */
//		Bios_locate(x.page_,x.vR1_+row,x.vC1_+col) ;
	} else
		p = p0 + (row * W + col) ;

	isC1 = 0 ;
	while ( (c=*s++) != 0 ) {
	    restart:
		switch( c ) {
		    case 0x07 : /* Bell */
			Bios_putc(7) ;
			break ;

		    case 0x08 : /* BackSpace */
			if ( --col < 0 )
				col = 0 ;
			else if ( !isBios(dir) )
				p-- ;
			break ;

		    case 0x09 : /* Tab */
			if ( isBios(dir) ) Bios_locate(x.page_,x.vR1_+row,x.vC1_+col) ;
			i = col ;
			col = (col+8) & 0xfff8 ;
			if ( col > w ) col = w ;

			i = col - i ;
			if ( i > 0 ) {
				if ( isBios(dir) ) {
					Bios_outc(' ',i,x.page_,x.attr_) ;
				} else {
					memsetW(p,Blank,i) ;
					p += i ;
				}
			}
			break ;

		    case 0x0a : /* LineFeed */
			if ( ++row >= h ) {
				row = h - 1 ;
				if ( isBios(dir) )
					Bios_scroll_up(1,x.attr_,x.vR1_,x.vC1_,
						       x.vR2_,x.vC2_) ;
				else
					mem_scroll_up(p0,W,w,h,1,Blank) ;
			}
			/* fall-thrugh next case */

		    case 0x0d : /* CarriageReturn */
			col = 0 ;
			if ( !isBios(dir) ) p = p0 + row * W ;
			break ;

		    default :
			if ( col >= w ) {
				if ( !x.wrap() ) {
					while ( (Byte)(*s) >= ' ' ) s++ ;
					continue ;
				}
				col = 0 ;
				if ( ++row >= h ) {
					row-- ;
					if ( x.oMode_ & oNoScrolling_ )
						; /* not scroll up */
					else if ( isBios(dir) )
						Bios_scroll_up(1,x.attr_,
							       x.vR1_,x.vC1_,
							       x.vR2_,x.vC2_) ;
					else
						mem_scroll_up(p0,W,w,h,1,Blank) ;

				}
				if ( !isBios(dir) ) p = p0 + row * W ;
			}
			if ( isBios(dir) ) Bios_locate(x.page_,x.vR1_+row,x.vC1_+col) ;

			col++ ;
			if ( isC1 )
				isC1 = 0 ;
			else if ( x.chinese() && c >= 128 ) {
				isC1 = 1 ;
				if ( col >= w ) {	/* spawn a line ? */
					if ( x.wrap() ) { /* back one char */
						s-- ;
					} else {	/* skip chinese char */
						if ( (Byte)(*s) > ' ' ) s++ ;
					}
					c = ' ' ;
					isC1 = 0 ;
				}
			}

			if ( isBios(dir) )
				Bios_outc(c,1,x.page_,x.attr_) ;
			else
				*p++ = c + ((int)x.attr_ << 8) ;

			break ;
		}
	}

	if ( !isBuffer(dir) ) {
		x.row_ = row ;
		x.col_ = (col > w ? w : col) ;

		if ( &x != x.who() ) {
			if ( isBios(dir) )
				x.who()->use() ; /* restore original cursor */
		} else if ( isBios(dir) ) {
			if ( isVisible(x.cType_) ) {
				Bios_set_cursor(x.cType_,x.cHigh_,x.cLow_) ;
				Bios_locate(x.page_,x.vR1_+row,
					    x.vC1_+(x.col_>=w?w-1:x.col_)) ;
			}
		} else if ( isVideo(dir) ) {
			if ( isVisible(x.cType_) )
				Bios_locate(x.page_,x.vR1_+row,
					    x.vC1_+(x.col_>=w?w-1:x.col_)) ;
		}
	}

#if defined(__DJGPP__)
	if ( isVideo(dir) ) DJNearptr_disable() ;
#endif
}

/*****************************************************************************/

twin::~twin ( )
{
	if ( this == current ) current = &stdscr ;
	if ( pBuffer_ ) delete[] pBuffer_ ;
}


twin::twin ( int row1, int col1, int row2, int col2 )
{
#if defined(__DJGPP__)
	if ( videoPtr == NULL ) {
		videoWidth = _farpeekb(_dos_ds,MK_FP(BIOS_SEG,VIDEO_WIDTH_OFS)) ;
	#if defined(VIDEO_HIGH_OFS)
		videoHigh = _farpeekb(_dos_ds,MK_FP(BIOS_SEG,VIDEO_HIGH_OFS))+1 ;
	#else
		videoHigh = MAX_ROW ;
	#endif
		videoMode = _farpeekb(_dos_ds,MK_FP(BIOS_SEG,VIDEO_MODE_OFS)) ;
		isColor   = videoMode != 0x07 ;
		videoPtr  = (Wfptr)( isColor ? MK_FP(COLOR_SEG,COLOR_OFS)
					     : MK_FP(MONO_SEG,MONO_OFS) ) ;
	#if !defined(PROTECT_DOS)
		__djgpp_nearptr_enable() ;
	#endif
	}
#endif
	pBuffer_ = 0 ;
	page_ = 0 ;
	attr_ = VIDEO_ATTR_NORMAL ;
	oMode_ = oWrap_ ;
	if ( directvideo )
		set_toVideo(1) ;
	else
		set_toBios(1) ;
	if ( ChineseSW ) set_chinese(1) ;

	gSecurity_ = 0 ;
	gMode_ = GMode ;

#if defined(__BORLANDC__)
	// BC++3.1: Object may be initialized before it's static instance !
	_R1_ = (videoHigh == 0 || 0 < row1 && row1 <= videoHigh ? row1-1 : 0) ;
	_C1_ = (videoWidth == 0 || 0 < col1 && col1 <= videoWidth ? col1-1 : 0) ;
	_R2_ = (videoHigh == 0 || _R1_ < row2 && row2 <= videoHigh
	       ? row2-1 : videoHigh -1) ;
	_C2_ = (videoWidth == 0 ||_C1_ < col2 && col2 <= videoWidth
	       ? col2-1 : videoWidth-1) ;
#else
	_R1_ = (0 < row1 && row1 <= videoHigh  ? row1-1 : 0) ;
	_C1_ = (0 < col1 && col1 <= videoWidth ? col1-1 : 0) ;
	_R2_ = (_R1_ < row2 && row2 <= videoHigh  ? row2-1 : videoHigh -1) ;
	_C2_ = (_C1_ < col2 && col2 <= videoWidth ? col2-1 : videoWidth-1) ;
#endif

	clr_view() ;

	{
		int	RowCol = Bios_get_locate(page_) ;
		row_ = RowCol >> 8 ;
		col_ = RowCol & 0xff ;
	}
	{
		int	Cursor = Bios_get_cursor() ;
		cType_ = (Cursor>>13) & 0x07 ;
		cHigh_ = (Cursor>>8 ) & 0x1f ;
		cLow_  = (Byte)Cursor ;
	}

	if ( row_ < vR1_ )
		row_ = vR1_ ;
	else if ( row_ > vR2_ )
		row_ = vR2_ ;

	if ( col_ < vC1_ )
		col_ = vC1_ ;
	else if ( col_ > vC2_ )
		col_ = vC2_ ;
}


void twin::set_toBuffer ( Bit onoff )
{
	if ( onoff )
		oMode_ |= oToBuffer_ ;
	else
		oMode_ &= ~oToBuffer_ ;
	if ( onoff && pBuffer_ == 0 ) {
		pBuffer_ = new Word[(_R2_-_R1_+1)*(_C2_-_C1_+1)] ;
		if ( pBuffer_ == 0 ) oMode_ &= ~oToBuffer_ ;
	}
}


twin * twin::use ( )
{
	twin	*last = current ;

	current = this ;
	if ( isVisible(cType_) ) {
		int	c = vC1_ + col_ ;
		if ( c >= videoWidth ) c = videoWidth - 1 ;
		Bios_locate(page_,vR1_+row_,c) ;
	}
	Bios_set_cursor(cType_,cHigh_,cLow_) ;

	return last ;
}


void twin::set_view ( int row1, int col1, int row2, int col2 )
{
	row1 += _R1_ - 1 ;
	vR1_ = (_R1_ <= row1 && row1 <= _R2_ ? row1 : _R1_) ;
	vR0_ = vR1_ - 1 ;

	row2 += _R1_ - 1 ;
	vR2_ = (vR1_ <= row2 && row2 <= _R2_ ? row2 : _R2_) ;

	col1 += _C1_ - 1 ;
	vC1_ = (_C1_ <= col1 && col1 <= _C2_ ? col1 : _C1_) ;
	vC0_ = vC1_ - 1 ;

	col2 += _C1_ - 1 ;
	vC2_ = (vC1_ <= col2 && col2 <= _C2_ ? col2 : _C2_) ;
}


void twin::subview ( int row1, int col1, int row2, int col2 )
{
	row1 += vR1_ ;
	vR1_ = (_R1_ <= row1 && row1 <= _R2_ ? row1 : _R1_) ;
	vR0_ = vR1_ - 1 ;

	row2 += vR2_ ;
	vR2_ = (vR1_ <= row2 && row2 <= _R2_ ? row2 : _R2_) ;

	col1 += vC1_ ;
	vC1_ = (_C1_ <= col1 && col1 <= _C2_ ? col1 : _C1_) ;
	vC0_ = vC1_ - 1 ;

	col2 += vC2_ ;
	vC2_ = (vC1_ <= col2 && col2 <= _C2_ ? col2 : _C2_) ;
}


void twin::set_cursor ( int type, int high, int low )
{
	if ( this == who() && cType_ != (Byte)type && !isVisible(cType_) )
		Bios_locate(page_,vR1_+row_,vC1_+col_) ;
	cType_ = (type & 0x07) ;
	cHigh_ = (high & 0x1f) ;
	cLow_  = low ;
	if ( this == who() ) Bios_set_cursor(cType_,cHigh_,cLow_) ;
}


void twin::locate ( int row, int col )
{
	int	r = row + vR0_ ;
	int	c = col + vC0_ ;

	if ( r > vR2_ ) r = vR2_ ;
	if ( c > vC2_ ) c = vC2_ ;

	if ( this == who() && isVisible(cType_) ) Bios_locate(page_,r,c) ;

	row_ = r - vR1_ ;
	col_ = c - vC1_ ;
}

/*****************************************************************************/

void twin::scroll_block_up ( int row1, int col1, int row2, int col2,
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
		mem_scroll_up(p,W,c2-c1+1,r2-r1+1,
			      numRow,' '+((int)attr_<<8)) ;
	}

	if ( toVideo() ) {
		Wfptr	p = videoPtr + (r1*videoWidth+c1) ;
#if defined(__DJGPP__)
		DJNearptr_enable() ;
		p = (Wfptr)((Bfptr)p + __djgpp_conventional_base) ;
#endif
		mem_scroll_up(p,videoWidth,c2-c1+1,r2-r1+1,
			      numRow,' '+((int)attr_<<8)) ;
#if defined(__DJGPP__)
		DJNearptr_disable() ;
#endif
	} else if ( toBios() ) {
		Bios_scroll_up(numRow,attr_,r1,c1,r2,c2) ;
	}
}

/*****************************************************************************/

void twin::outs ( const void *str )
{
	if ( toVideo() || toBios() )
		twin_outs(*this,(char*)str,toVideo()?TO_VIDEO:TO_BIOS) ;
	if ( toBuffer() ) twin_outs(*this,(char*)str,TO_BUFFER) ;
}


int twin::putch ( int ch )
{
	ch &= 0xff ;
	outs((const char*)&ch) ;
	return ch ;
}


void twin::outc ( int ch, int len )
{
	int	r, c	;
	char	s[160]	;
	int	oldMode ;

	r = row_ ; c = col_ ;
	oldMode = oMode_ ;
	oMode_ |= oNoScrolling_ ;

	if ( len >= sizeof(s) ) len = sizeof(s) - 1 ;
	memset(s,ch,len) ;
	s[len] = '\0' ;
	outs(s) ;

	oMode_ = oldMode ;
	row_ = r ; col_ = c ;
	if ( isVisible(cType_) ) Bios_locate(page_,vR1_+r,vC1_+c) ;
}

/*****************************************************************************/

