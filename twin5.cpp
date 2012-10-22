/*
 * $Log: TWIN5.CPP $
 * Revision 1.1  1996-02-27 20:42:34-0800  ycchen
 * (1) Change frame[] type of window_[c]frame() to void *
 * (2) Fixed position of frame[3] & frame[7]
 *
 * Revision 1.0  1996-02-06 23:29:13-0800  YCCHEN
 * Initial revision
 *
**/

#include	<string.h>
#include	"c0.h"
#include	"inkey.h"
#include	"twin.h"


void window_view ( twin *w )
{
	if ( w == 0 ) w = &stdscr ;
	w->clr_view() ;
	w->use() ;
}


void window_subview ( twin *w, int r1, int c1, int r2, int c2 )
{
	if ( w == 0 ) w = &stdscr ;
	w->subview(r1,c1,r2,c2) ;
	w->use() ;
}


void window_frame ( twin *w, void *Frame )
{
	int	i, j	;
	int	r, c	;
	int	oMode	;
	Bptr	frame = (Bptr)Frame ;

	if ( strlen((char*)frame) < 8 ||
	     (r=w->screen_high()) < 2 ||
	     (c=w->screen_width()) < 2	 ) return ;

	oMode = w->outMode() ;
	w->clr_view() ;
	w->use() ;
	w->set_wrap(0) ;
	w->set_chinese(0) ;

	w->locate(1,1) ; w->outc(frame[0],1) ;
	if ( c > 2 ) {
		w->locate(1,2) ; w->outc(frame[1],c-2) ;
		w->locate(r,2) ; w->outc(frame[5],c-2) ;
	}
	w->locate(1,c) ; w->outc(frame[2],1) ;
	if ( r > 2 ) {
		for ( i = 2 ; i < r ; i++ ) {
			w->locate(i,1) ; w->outc(frame[7],1) ;
			w->locate(i,c) ; w->outc(frame[3],1) ;
		}
	}
	w->locate(r,1) ; w->outc(frame[6],1) ;
	w->locate(r,c) ; w->outc(frame[4],1) ;
	w->subview(1,1,-1,-1) ;
	w->set_outMode(oMode) ;
}


static void outc2 ( int r, int c, Word x, int n )
{
	Word	s[50]	;
	int	i	;

	locate(r,c) ;
	for ( i = 0 ; i < n ; i++ ) s[i] = x ;
	s[n] = 0 ;
	outs(s) ;
}


void window_cframe ( twin *w, void *Frame )
{
	int	i, j	;
	int	r, c	;
	int	wrap	;
	Wptr	frame = (Wptr)Frame ;

	if ( strlen((char*)frame) < 16 ||
	     (r=w->screen_high()) < 2  ||
	     (c=w->screen_width()-1) < 4  ) return ;

	wrap = w->wrap() ;
	w->clr_view() ;
	w->use() ;
	w->set_wrap(0) ;

	outc2(1,1,frame[0],1) ;
	if ( c > 4 ) {
		outc2(1,3,frame[1],(c-2)>>1) ;
		outc2(r,3,frame[5],(c-2)>>1) ;
	}
	outc2(1,c,frame[2],1) ;
	if ( r > 2 ) {
		for ( i = 2 ; i < r ; i++ ) {
			outc2(i,1,frame[7],1) ;
			outc2(i,c,frame[3],1) ;
		}
	}
	outc2(r,1,frame[6],1) ;
	outc2(r,c,frame[4],1) ;
	w->subview(1,2,-1,-2) ;
	w->set_wrap(wrap) ;
}


int window_browse ( twin *w, int light_bar, IfuncI input, int input_list[],
		    IfuncC initial, IfuncI get_data, VfuncI show_data,
		    VfuncI move_ptr, char *title )
{
	int	total_line, current_line ;
	int	last_line, bottom_line ;
	int	i, k, m ;
	int	*pi	;

	w->use() ;
	k = (initial)(title) ;
	total_line = w->display_high() ;
	for ( bottom_line = 0 ; bottom_line < total_line ; ) {
		if ( (get_data)(bottom_line) != 0 ) break ;
		bottom_line++ ;
		if ( k ) (show_data)(bottom_line) ;
	}

	current_line = light_bar > 0 ? light_bar - 1 : 0 ;
	for ( ; ; ) {
		if ( light_bar > 0 ) {
			Byte	a = w->attr() ;
			w->set_attr((a>>4)|(a<<4)) ;
			(get_data)(current_line) ;
			(show_data)(current_line+1) ;
			w->set_attr(a) ;
		}
		last_line = current_line ;

		while ( current_line == last_line ) {
			k = (input)(current_line) ;
			if ( k == 0 ) break ;	/* redisplay light_bar */
			for ( i=-1, pi=input_list ; *pi != 0 ; i--, pi++ ) {
				if ( k == *pi ) return i ;
			}
			switch( k ) {
			    case A_HOME :
				for ( bottom_line = 0 ; bottom_line < total_line ; ) {
					if ( (get_data)(bottom_line) != 0 ) break ;
					bottom_line++ ;
					(show_data)(bottom_line) ;
				}
				if ( bottom_line < total_line )
					clr_scr(bottom_line+1,total_line) ;
				if ( current_line >= bottom_line )
					current_line = bottom_line -1 ;
				last_line = -1 ;
				break ;
			    case CR :
				return current_line + 1 ;
			    case ESC :
			    case -1 :
				return 0 ;
			    case K_HOME :
				current_line = 0 ;
				break ;
			    case K_END :
				current_line = bottom_line - 1 ;
				break ;
			    case K_UP :
				if ( current_line > 0 ) {
					current_line-- ;
					break ;
				}
				if ( (get_data)(-1) != 0 ) {
					w->putch(BEL) ;
					break ;
				}
				if ( light_bar > 0 ) last_line++ ;
				scroll_down(1,25,1) ;
				(show_data)(1) ;
				(move_ptr)(-1) ;
				if ( bottom_line < total_line ) bottom_line++ ;
				break ;
			    case K_DOWN :
				if ( light_bar>0 && current_line+1<total_line ) {
					if ( (get_data)(current_line+1) != 0 ) {
						if ( current_line + 1 < bottom_line )
							bottom_line = current_line + 1 ;
						w->putch(BEL) ;
						break ;
					}
					current_line++ ;
					if ( current_line >= bottom_line )
						bottom_line = current_line + 1 ;
				} else if ( (get_data)(total_line) == 0 ) {
					scroll_up(1,25,1) ;
					(show_data)(total_line) ;
					(move_ptr)(1) ;
					if ( light_bar > 0 ) last_line-- ;
				} else {
					w->putch(BEL) ;
				}
				break ;
			    case K_PGUP :
				if ( (get_data)(-1) != 0 ) {
					w->putch(BEL) ;
					break ;
				}
				for ( m = total_line ; m > 1 ; m-- ) {
					if ( (get_data)(-m) == 0 ) break ;
				}
				/* backward m records */
				if ( m < total_line ) scroll_down(1,25,m) ;
				(move_ptr)(-m) ;
				if ( light_bar > 0 ) last_line += m ;
				for ( i = 0 ; i < m ; ) {
					(get_data)(i) ;
					(show_data)(++i) ;
				}
				bottom_line += m ;
				if ( bottom_line > total_line )
					bottom_line = total_line ;
				break ;
			    case K_PGDN :
				if ( (get_data)(total_line) != 0 ) {
					w->putch(BEL) ;
					break ;
				}
				(move_ptr)(total_line) ;
				for ( i = 0 ; i < total_line ; ) {
					if ( (get_data)(i) != 0 ) break ;
					(show_data)(++i) ;
				}
				bottom_line = i ;
				if ( i < total_line ) clr_scr(i+1,25) ;
				if ( light_bar > 0 ) {
					last_line -= total_line ;
					if ( current_line >= i )
						current_line = i - 1 ;
				}
				break ;
			    default :
				w->putch(BEL) ;
				break ;
			}
		}
		if ( last_line >= 0 && last_line < total_line ) {
			(get_data)(last_line) ;
			(show_data)(last_line+1) ;
		}
	}
}

