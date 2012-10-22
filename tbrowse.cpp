/*
 * $Log: tbrowse.cpp $
 *
**/

#include "tbrowse.h"

Tbrowse::Tbrowse ( int r1, int c1, int r2, int c2 ) : twin(r1,c1,r2,c2)
{
	_pos = _total = 0 ;
	_nth = 0 ;
	_screenHigh = display_high() ;
	_cNor = attr() ;
	_cBar = is_color_mode() ? 0x2e : 0x70 ;
}


void Tbrowse::refresh ( int show )
{
	int	i	;

	if ( show ) set_attr(_cNor) ;
	for ( i = 0 ; i < _screenHigh && getData(i) ; i++ ) {
		if ( show ) showData(i) ;
	}
	_visibleHigh = i ;
	if ( show && _visibleHigh < _screenHigh )
		clr_scr(_visibleHigh+1,_screenHigh) ;

	if ( _nth >= _visibleHigh ) _nth = _visibleHigh - 1 ;
}


int Tbrowse::browse ( int bar, const char *title )
{
	int	i, k, m ;
	int	nth	;

	use() ;
	if ( bar >= 0 ) refresh(initial(title)) ;

	if ( bar > 0 ) _nth = bar - 1 ;
	for ( ; ; ) {
		if ( bar ) {
			set_attr(_cBar) ;
			getData(_nth) ;
			showData(_nth) ;
			set_attr(_cNor) ;
		}

		nth = _nth ;
		while ( _nth == nth ) {
			k = input(nth) ;
			if ( k == 0 ) break ;	/* redisplay light_bar */

			switch( k ) {
			    case A_HOME : // refresh
				refresh(1) ;
				nth = -1 ;
				break ;
			    case CR :
				return bar ? _nth + 1 : 1 ;
			    case ESC :
			    case -1 :
				return 0 ;
			    case K_HOME :
				if ( bar && _nth != 0 ) {
					showData(_nth) ;
					_nth = 0 ;
				}
				break ;
			    case K_END :
				if ( bar && _nth+1 != _visibleHigh ) {
					showData(_nth) ;
					_nth = _visibleHigh - 1 ;
				}
				break ;
			    case K_UP :
				if ( bar && _nth > 0 ) {
					showData(_nth) ;
					_nth-- ;
					break ;
				}
				if ( !getData(-1) ) {
					putch(BEL) ;
					break ;
				}
				if ( bar ) {
					showData(_nth) ;
					nth++ ;   // so (_nth <> nth)
				}
				scroll_down(1,_screenHigh,1) ;
				moveData(-1) ;
				if ( _visibleHigh < _screenHigh ) _visibleHigh++ ;
				break ;
			    case K_DOWN :
				if ( bar && _nth+1 < _screenHigh ) {
					if ( !getData(_nth+1) ) {
						if ( _nth+1 < _visibleHigh )
							_visibleHigh = _nth + 1 ;
						putch(BEL) ;
						break ;
					}
					showData(_nth) ;
					if ( ++_nth >= _visibleHigh )
						_visibleHigh = _nth + 1 ;
				} else if ( getData(_screenHigh) ) {
					if ( bar ) {
						showData(_nth) ;
						nth-- ;
					}
					scroll_up(1,_screenHigh,1) ;
					moveData(1) ;
				} else {
					putch(BEL) ;
				}
				break ;
			    case K_PGUP :
				if ( !getData(-1) ) {
					putch(BEL) ;
					break ;
				}
				if ( bar ) showData(_nth) ;
				for ( m = _screenHigh ; m > 1 ; m-- ) {
					if ( getData(-m) ) break ;
				}
				/* backward m records */
				if ( m < _screenHigh ) scroll_down(1,_screenHigh,m) ;
				if ( bar ) nth += m ;
				moveData(-m) ;
				for ( i = 0 ; i < m ; i++ ) {
					getData(i) ;
					showData(i) ;
				}
				_visibleHigh += m ;
				if ( _visibleHigh > _screenHigh )
					_visibleHigh = _screenHigh ;
				break ;
			    case K_PGDN :
				if ( !getData(_screenHigh) ) {
					putch(BEL) ;
					break ;
				}
				moveData(_screenHigh) ;
				refresh(1) ;
				nth = -1 ;
				break ;
			    default :
				putch(BEL) ;
				break ;
			}
		}
	}
}

