/* test.cpp --
*/

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	"twin.h"
#include	"inkey.h"


void test1 ( twin &X, twin &L, twin &R, long cnt )
{
	int	key	;
	Byte	ch	;

	X.locate(1,1) ; X.set_attr(0x02) ; X.clr_scr() ;
	L.locate(1,1) ; L.set_attr(0x03) ; L.clr_scr() ;
	R.locate(1,1) ; R.set_attr(0x04) ; R.clr_scr() ;

	ch = 0 ;
	for ( ; cnt > 0 ; cnt-- ) {
#if !defined(NO_IN_KEY)
		if ( in_key() != NOKEY ) break ;
#endif
		ch++ ;
		X.outs("\n") ; X.set_attr(ch) ; X.outc(ch<32?'.':ch,80) ;
		ch++ ;
		L.outs("\n") ; L.set_attr(ch) ; L.outc(ch<32?'.':ch,40) ;
		ch++ ;
		R.outs("\n") ; R.set_attr(ch) ; R.outc(ch<32?'.':ch,40) ;
	}
}


void showMsg ( twin &X, char *msg, int attr )
{
	int	r1, c1, r2, c2	;

	X.get_view(&r1,&c1,&r2,&c2) ;
	X.set_attr(attr^0xff) ;
	X.clr_scr() ;

	int	len = strlen(msg) ;
	X.set_attr(attr) ;
	X.locate((r2-r1)/2,(c2-c1+1-len)/2) ;
	X.outs(msg) ;
}


void test2 ( twin &X, twin &L, twin &R, long cnt )
{
	Byte	ch = 0	;

	for ( ; cnt > 0 ; cnt-- ) {
#if !defined(NO_IN_KEY)
		if ( in_key() != NOKEY ) break ;
#endif
		showMsg(X," T E S T - 1 ",ch++) ;
		showMsg(L,"  T E S T -- 2  ",ch++) ;
		showMsg(R,"   ¤¤¤å´ú¸Õ --- 3   ",ch++) ;
	}
}


void test3 ( twin &X, twin &L, twin &R, long cnt )
{
	int	key	;
	Byte	ch	;

	X.locate(1,1) ; X.set_attr(0x02) ; X.clr_scr() ;
	L.locate(1,1) ; L.set_attr(0x03) ; L.clr_scr() ;
	R.locate(1,1) ; R.set_attr(0x04) ; R.clr_scr() ;

	ch = 0 ;
	for ( ; cnt > 0 ; cnt-- ) {
#if !defined(NO_IN_KEY)
		if ( in_key() != NOKEY ) break ;
#endif
		ch++ ;
		X.set_attr(ch) ; X.printf("Window 1: %u\n",ch) ;
		ch++ ;
		L.set_attr(ch) ; L.printf("Window 2: %u\n",ch) ;
		ch++ ;
		R.set_attr(ch) ; R.printf("Window 3: %u\n",ch) ;
	}
}


main ( int argc, char *argv[] )
{
	long	cnt = 1000L	;
	int	needPause = 0	;

	for ( int i = 1 ; i < argc ; i++ ) {
		switch( argv[i][0] ) {
		    case 'P' :
		    case 'p' :
			needPause = 1 ;
			break ;

		    case 'V' :
		    case 'v' :
			directvideo = 1 ;
			break ;

		    case 'B' :
		    case 'b' :
			directvideo = 0 ;
			break ;

		    default :
			if ( argv[i][0] >= '0' && argv[i][0] <= '9' )
				cnt = atol(argv[i]) ;
			break ;
		}
	}

	stdscr.cursor_off() ;
	stdscr.set_toVideo(directvideo) ;
	stdscr.set_toBios(directvideo^1) ;
//	stdscr.set_attr(0x10) ;
	       set_attr(0x10) ;
//	stdscr.clr_scr() ;
	       clr_scr() ;

    {
	twin	X( 1, 1,10,80) ;
	twin	L(11, 1,20,40) ;
	twin	R(11,41,20,80) ;

	test1(X,L,R,cnt) ;
	X.save_image() ; L.save_image() ; R.save_image() ;
	test2(X,L,R,cnt) ;
	test3(X,L,R,cnt) ;

//	stdscr.set_attr(0x07) ;
	       set_attr(0x07) ;
	if ( needPause ) {
//		stdscr.clr_scr(24,25) ;
		       clr_scr(24,25) ;
//		stdscr.locate(24,1) ;
		       locate(24,1) ;
//		stdscr.outs(">>>> press any key to continue ...") ;
		       outs(">>>> press any key to continue ...") ;
		clr_key() ;
		wait_key() ;
	}

	X.restore_image() ; L.restore_image() ; R.restore_image() ;

	if ( needPause ) {
//		stdscr.clr_scr(24,25) ;
		       clr_scr(24,25) ;
//		stdscr.locate(24,1) ;
		       locate(24,1) ;
//		stdscr.outs(">>>> press any key to continue ...") ;
		       outs(">>>> press any key to continue ...") ;
		clr_key() ;
		wait_key() ;
	}
    }

    {
	twin	A( 6,28,10,47) ;
	twin	B( 8,31,12,50) ;
	twin	C(10,34,14,53) ;
	Wfptr	p[3] ;
	p[0] = A.save_screen() ;
	p[1] = B.save_screen() ;
	p[2] = C.save_screen() ;
	int	k = 0 ;
	for ( int n = 0 ; n < 100 ; n++ ) {
		for ( int i = 1 ; i <= 20 ; i += 5 ) {
			for ( int j = 1 ; j <= 80 ; j += 20 ) {
				twin	D(i,j,i+4,j+19) ;
				D.restore_screen(p[k]) ;
				k = (k+1) % 3 ;
			}
		}
	}
     }

	stdscr.cursor_on() ;
//	stdscr.clr_scr(24,25) ;
	       clr_scr(24,25) ;
//	stdscr.locate(24,1) ;
	       locate(24,1) ;
//	stdscr.outs(">>>> end of testing ...") ;
	       outs(">>>> end of testing ...") ;
	clr_key() ;

	return 0 ;
}

