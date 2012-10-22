/*
 * $Log: TWIN.H $
 * Revision 1.1  1996-02-27 20:41:01-0800  ycchen
 * Change frame[] type of window_[c]frame() to void *
 *
 * Revision 1.0  1996-02-06 23:29:14-0800  YCCHEN
 * Initial revision
 *
**/

#if !defined(_TWIN_H_)
#define _TWIN_H_

#include	"typedef.h"

#if defined(__DJGPP__)
    #if defined(PROTECT_DOS)
	#include <sys/nearptr.h>
	#define DJNearptr_enable()	__djgpp_nearptr_enable() ;
	#define DJNearptr_disable()	__djgpp_nearptr_disable() ;
    #else
	#define DJNearptr_enable()	NIL
	#define DJNearptr_disable()	NIL
    #endif
#endif

#define oWrap_		0x01	/* line wrap			      */
#define oChinese_	0x02	/* chinese display		      */
#define oToBuffer_	0x04	/* direct to pBuffer_[] 	      */
#define oToVideo_	0x08	/* write to videoPtr[]		      */
#define oToBios_	0x10	/* write via BIOS		      */
#define oNoScrolling_	0x20	/* not scrolling when out of screen   */

#define gUpdated_	0x01	/* string changed after get function  */
#define gConfirm_	0x02	/* L/R arrow, ascii over string       */
#define gKeepIns_	0x04	/* keep insert mode		      */
#define gIsIns_ 	0x08	/* last insert status		      */
#define gDoSpecial_	0x10	/* enable HOME/END to first/last pos. */
#define gSkipBlank_	0x20	/* skip leading blank		      */
#define gShiftLeft_	0x40	/* enable input at end of field       */
#define gBinActive_	0x80	/* active SkipBlank/ShiftLeft if bin. */

#ifndef _FUNCx_
#define _FUNCx_
typedef int	(*IfuncI)( int )		;
typedef int	(*IfuncC)( char * )		;
typedef void	(*VfuncI)( int )		;
#endif

class	twin	{
	// twin.cpp
	static	Byte	videoMode	;
	static	Byte	isColor 	;
	static	Wfptr	videoPtr	;
	static	int	videoWidth	;
	static	int	videoHigh	;
	static	twin	*current	;

    private:
	Wptr	pBuffer_	; /* my screen image			*/
	int	_R1_, _C1_	, /* real left-upper (count from 0)	*/
		_R2_, _C2_	; /* real right-down (count from 0)	*/
	int	vR1_, vC1_	, /* view left-upper (count from 0)	*/
		vR2_, vC2_	; /* view right-down (count from 0)	*/
	int	vR0_		, /* == vR1_ - 1			*/
		vC0_		; /* == vC1_ - 1			*/
	int	row_		, /* current (row,col) relative to	*/
		col_		; /*	 (vR1_,vC1) (count from 0)	*/
	Byte	attr_		; /* current attribute			*/
	Byte	page_		; /* screen page of BIOS		*/
	Byte	cType_		, /* cursor type			*/
		cHigh_		, /* cursor shape: high position	*/
		cLow_		; /* cursor shape: low position 	*/
	Byte	oMode_		;
	Byte	gMode_		;
	Byte	gSecurity_	; /* display char for get function	*/

	/* twin.cpp */
	friend void twin_outs ( twin &x, const char *s, int dir )	;

    public:
	// twin.cpp
	~twin( )							;
	twin( int row1=0, int col1=0, int row2=0, int col2=0 )		;
	void set_toBuffer( Bit onoff )					;
	twin *use( )							;
	void subview( int row1, int col1, int row2, int col2 )		;
	void set_view( int row1, int col1, int row2, int col2 ) 	;
	void set_cursor( int type, int high, int low )			;
	void locate( int row, int col ) 				;
	void scroll_block_up( int row1, int col1,
			      int row2, int col2, int numRow )		;
	void outs( const void *str )					;
	int  putch( int ch )						;
	void outc( int ch, int len )					;

	// twin1.cpp
	void scroll_block_down( int row1, int col1,
				int row2, int col2, int numRow )	;

	// twin2.cpp
	Wfptr save_screen( )						;
	void restore_screen( Wfptr screen, int dofree=0 )		;
	void free_image( )						;
	Wptr save_image( )						;
	void restore_image( int dofree=0 )				;

	// twin3.cpp
	int printf( const char *fmt, ... )				;

	// twin4.cpp
	int getca( int row=0, int col=0 )				;

	// twinget.cpp
	int get( int row, int col, char str[], int pos=0 )		;
	int get( char str[], int pos=0 )				;

    // inline functions

	twin * who ( )
	{
		return current ;
	}

	int is_color_mode ( void )
	{
		return isColor ;
	}

	Wfptr get_video_ptr ( void )
	{
		return videoPtr ;
	}

	int row_based ( void )
	{
		return _R1_ + 1 ;
	}

	int col_based ( void )
	{
		return _C1_ + 1 ;
	}

	int screen_high ( void )
	{
		return _R2_ - _R1_ + 1 ;
	}


	int screen_width ( void )
	{
		return _C2_ - _C1_ + 1 ;
	}


	int display_high ( void )
	{
		return vR2_ - vR0_ ;
	}


	int display_width ( void )
	{
		return vC2_ - vC0_ ;
	}

	void set_page ( int page )
	{
		page_ = page ;
	}

	Byte page ( )
	{
		return page_ ;
	}

	void set_attr ( int attr )
	{
		attr_ = attr ;
	}

	Byte attr ( )
	{
		return attr_ ;
	}

	void cursor_on ( )
	{
		set_cursor(0,cHigh_,cLow_) ;
	}

	void cursor_off ( )
	{
		set_cursor(1,cHigh_,cLow_) ;
	}

	int row ( )
	{
		return row_+1 ;
	}

	int col ( )
	{
		return col_+1 ;
	}

	void get_locate ( int *row, int *col )
	{
		*row = row_+1 ;
		*col = col_+1 ;
	}

	void get_cursor ( int *type, int *high, int *low )
	{
		*type = cType_ ;
		*high = cHigh_ ;
		*low  = cLow_ ;
	}

	void clr_view ( )
	{
		vR1_ = _R1_ ;
		vR0_ = _R1_ - 1 ;
		vR2_ = _R2_ ;

		vC1_ = _C1_ ;
		vC0_ = _C1_ - 1 ;
		vC2_ = _C2_ ;
	}

	void get_view ( int *row1, int *col1, int *row2, int *col2 )
	{
		*row1 = vR1_ - _R1_ + 1 ;
		*col1 = vC1_ - _C1_ + 1 ;
		*row2 = vR2_ - _R1_ + 1 ;
		*col2 = vC2_ - _C1_ + 1 ;
	}

	void clr_scr ( int row1=1, int row2=100 )
	{
		scroll_block_up(row1,1,row2,vC2_+1,0) ;
	}

	void clr_block ( int row1, int col1, int row2, int col2 )
	{
		scroll_block_up(row1,col1,row2,col2,0) ;
	}

	void scroll_up ( int row1, int row2, int numRow )
	{
		scroll_block_up(row1,1,row2,vC2_+1,numRow) ;
	}

	void scroll_down ( int row1, int row2, int numRow )
	{
		scroll_block_down(row1,1,row2,vC2_+1,numRow) ;
	}

    // outMode ...

	void set_outMode ( Byte x )
	{
		oMode_ = x ;
	}

	Byte outMode ( )
	{
		return oMode_ ;
	}

	void set_wrap ( Bit onoff )
	{
		if ( onoff )
			oMode_ |= oWrap_ ;
		else
			oMode_ &= ~oWrap_ ;
	}

	Bit wrap ( )
	{
		return oMode_ & oWrap_ ;
	}

	void set_chinese ( Bit onoff )
	{
		if ( onoff )
			oMode_ |= oChinese_ ;
		else
			oMode_ &= ~oChinese_ ;
	}

	Bit chinese ( )
	{
		return oMode_ & oChinese_ ;
	}

	Bit toBuffer ( )
	{
		return oMode_ & oToBuffer_ ;
	}

	void set_toVideo ( Bit onoff )
	{
		if ( onoff )
			oMode_ |= oToVideo_ ;
		else
			oMode_ &= ~oToVideo_ ;
	}

	Bit toVideo ( )
	{
		return oMode_ & oToVideo_ ;
	}

	void set_toBios ( Bit onoff )
	{
		if ( onoff )
			oMode_ |= oToBios_ ;
		else
			oMode_ &= ~oToBios_ ;
	}

	Bit toBios ( )
	{
		return oMode_ & oToBios_ ;
	}

    // getMode ...

	void set_getMode ( Byte x )
	{
		gMode_ = x ;
	}

	Byte getMode ( )
	{
		return gMode_ ;
	}

	void set_getConfirm ( Bit onoff )
	{
		if ( onoff )
			gMode_ |= gConfirm_ ;
		else
			gMode_ &= ~gConfirm_ ;
	}

	Bit getConfirm ( )
	{
		return gMode_ & gConfirm_ ;
	}

	void set_getSecurity ( Byte ch )
	{
		gSecurity_ = ch ;
	}

	Byte getSecurity ( )
	{
		return gSecurity_ ;
	}

	void set_getKeepIns ( Bit onoff )
	{
		if ( onoff )
			gMode_ |= gKeepIns_ ;
		else
			gMode_ &= ~gKeepIns_ ;
	}

	Bit getKeepIns ( )
	{
		return gMode_ & gKeepIns_ ;
	}

	void set_getDoSpecial ( Bit onoff )
	{
		if ( onoff )
			gMode_ |= gDoSpecial_ ;
		else
			gMode_ &= ~gDoSpecial_ ;
	}

	Bit getDoSpecial ( )
	{
		return gMode_ & gDoSpecial_ ;
	}

	void set_getShiftLeft ( Bit onoff )
	{
		if ( onoff )
			gMode_ |= gShiftLeft_ ;
		else
			gMode_ &= ~gShiftLeft_ ;
	}

	Bit getShiftLeft ( )
	{
		return gMode_ & gShiftLeft_ ;
	}

	void set_getSkipBlank ( Bit onoff )
	{
		if ( onoff )
			gMode_ |= gSkipBlank_ ;
		else
			gMode_ &= ~gSkipBlank_ ;
	}

	Bit getSkipBlank ( )
	{
		return gMode_ & gSkipBlank_ ;
	}

	void set_getBinActive ( Bit onoff )
	{
		if ( onoff )
			gMode_ |= gBinActive_ ;
		else
			gMode_ &= ~gBinActive_ ;
	}

	Bit getBinActive ( )
	{
		return gMode_ & gBinActive_ ;
	}

	Bit getUpdated ( )
	{
		return gMode_ & gUpdated_ ;
	}
}	;

// twin.cpp
extern	twin	stdscr		;
extern	int	ChineseSW	; /* check Chinese on line boundary	*/
extern	int	GMode		;
extern	int	directvideo	;

#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************************/

// twin3.cpp
extern	int xprintf( const char *fmt, ... )				;

// twin5.cpp (window)
extern	void window_view ( twin *w )					;
extern	void window_subview ( twin *w, int r1, int c1, int r2, int c2 ) ;
extern	void window_frame ( twin *w, void *frame )			;
extern	void window_cframe ( twin *w, void *frame )			;
extern	int window_browse ( twin *w, int light_bar, IfuncI input, int input_list[],
			    IfuncC initial, IfuncI get_data, VfuncI show_data,
			    VfuncI move_ptr, char *title = 0 )		 ;

/*****************************************************************************/

inline void set_wrap ( int onoff )
{
	(stdscr.who())->set_wrap(onoff) ;
}


inline void set_attr ( int attr )
{
	(stdscr.who())->set_attr(attr) ;
}


inline void clr_scr ( int row1=1, int row2=100 )
{
	(stdscr.who())->clr_scr(row1,row2) ;
}


inline void cursor_on ( )
{
	(stdscr.who())->cursor_on() ;
}


inline void cursor_off ( )
{
	(stdscr.who())->cursor_off() ;
}


inline void set_cursor ( int type, int high, int low )
{
	(stdscr.who())->set_cursor(type,high,low) ;
}


inline void get_cursor ( int *type, int *high, int *low )
{
	(stdscr.who())->get_cursor(type,high,low) ;
}


inline void locate ( int row, int col )
{
	(stdscr.who())->locate(row,col) ;
}


inline void get_locate ( int *row, int *col )
{
	(stdscr.who())->get_locate(row,col) ;
}


inline void outs ( const void *str )
{
	(stdscr.who())->outs((char*)str) ;
}


inline void outc ( int ch, int len )
{
	(stdscr.who())->outc(ch,len) ;
}


inline void scroll_up ( int row1, int row2, int numRow )
{
	(stdscr.who())->scroll_up(row1,row2,numRow) ;
}


inline void scroll_down ( int row1, int row2, int numRow )
{
	(stdscr.who())->scroll_down(row1,row2,numRow) ;
}


inline int get_string ( int row, int col, char str[] )
{
	return (stdscr.who())->get(row,col,str) ;
}


inline int get_string0 ( int row, int col, char str[], int pos )
{
	return (stdscr.who())->get(row,col,str,pos) ;
}


inline twin * window_where ( void )
{
	return stdscr.who() ;
}


inline twin * window_create ( int r1, int c1, int r2, int c2 )
{
	return new twin(r1,c1,r2,c2) ;
}


inline void window_use ( twin *w )
{
	if ( w )
		w->use() ;
	else
		stdscr.use() ;
}


inline void window_close ( twin *w )
{
	if ( w ) delete w ;
}


inline void window_clear ( twin *w )
{
	if ( w )
		w->clr_scr() ;
	else
		stdscr.clr_scr() ;
}


inline void window_save ( twin *w )
{
	if ( w )
		w->save_image() ;
	else
		stdscr.save_image() ;
}


inline void window_restore ( twin *w )
{
	if ( w )
		w->restore_image() ;
	else
		stdscr.restore_image() ;
}

/*****************************************************************************/

#ifdef __cplusplus
};
#endif

#endif	/* _TWIN_H_ */

