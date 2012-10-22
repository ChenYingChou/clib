/*
 * $Log: TWINGET.CPP $
 * Revision 1.0  1996-02-06 23:29:13-0800  YCCHEN
 * Initial revision
 *

   int twin::get( int row, int col, char str[], int pos=0 )
   int twin::get( char str[], int pos=0 )

*/

#include	<string.h>
#include	"chinese.h"
#include	"inkey.h"

#define 	_TWIN_
#include	"twin.h"

#define 	BEL		0x07
#define 	BS		0x08
#define 	HT		0x09
#define 	TAB		HT
#define 	LF		0x0a
#define 	FF		0x0c
#define 	CR		0x0d
#define 	ESC		0x1b
#define 	SP		0x20
#define 	DEL		0x7f

/*****************************************************************************/

/* insert c at ith position of string s */
static int Insert ( twin *cur, char s[], int l, int n, int c )
{
	int	i, j, k ;

	if ( cur->chinese() && is_chinese2(s,n) ) n++;
	if ( n >= l ) {
		cur->putch(BEL) ;
		return n ;
	}

	i = l - 1 ;
	if ( c & 0xff00 ) {	/* chinese code */
		if ( n >= i ) { /* only one space to be inserted */
			cur->putch(BEL) ;
			return n ;
		}
		j = i - 2 ;
	} else
		j = i - 1 ;

	while ( j >= n )
		s[i--] = s[j--] ;

	if ( c & 0xff00 ) s[n++] = c >> 8 ;
	s[n++] = c ;

	if ( cur->chinese() && is_chinese2(s,l) ) s[l-1] = ' ' ;
	return n ;
}


/* replace s[n] with c */
static int Replace ( twin *cur, char s[], int l, int n, int c )
{
	if ( n >= l ) {
		cur->putch(BEL) ;
		return n ;
	}

	if ( cur->chinese() ) {
		if ( is_chinese2(s,n) ) { /* replace 2nd byte of chinese code */
			s[n-1] = ' ' ;
			s[n] = ' ' ;
		}
		if ( n+1<l && is_chinese2(s,n+1) ) { /* replace 1st byte of chinese code */
			s[n] = ' ' ;
			s[n+1] = ' ' ;
		}

		if ( c & 0xff00 ) {
			if ( n+1 >= l ) {	/* 2nd byte out of string length */
				cur->putch(BEL) ;
				return n ;
			}
			if ( n+2<l && is_chinese2(s,n+2) ) s[n+2] = ' ' ;
			s[n++] = c >> 8 ;
		}
	}

	s[n++] = c ;
	return n ;
}


/* delete s[n] */
static int Delete ( twin *cur, char s[], int l, int n )
{
	int	i;

	if ( cur->chinese() ) {
		if ( is_chinese2(s,n) ) n-- ;
		i = is_chinese2(s,n+1) ? n + 2 : n + 1 ;
	} else
		i = n + 1 ;

	while ( i < l ) s[n++] = s[i++] ;
	s[n++] = ' ' ;
	if ( n >= l ) return 1 ;	/* delete single byte */

	s[n] = ' ' ;
	return 2 ;	/* delete double byte */
}


int twin::get ( int il, int ic, char s[], int pos )
{
	int	i, j, c, l	;
	int	ct, ch, cl	;
	int	ctype, clow	;
	int	isIns, isChange ;
	int	atEnd		;

	gMode_ &= ~gUpdated_ ;

	l = strlen(s) ;
	isIns = (getKeepIns() ? (gMode_ & gIsIns_) : 0) ;

	get_cursor(&ct,&ch,&cl) ;
	ctype = ct & ~0x03 ;
	clow  = (cl < 7) ? 13 : cl ;

	if ( il == 0 ) il = row() ;
	if ( ic == 0 ) ic = col() ;

	set_cursor(ctype,isIns?clow>>1:0,clow) ;

	i = pos ;
	if ( getSkipBlank() ) {
		while ( i < l && (s[i] == SP || s[i] == HT) ) i++ ;
	}
	if ( i >= l ) i = 0 ;

	c = CR ;
	isChange = -1 ;
	atEnd = 0 ;
	for ( ; ; ) {
		if ( isChange ) {
			if ( isChange > 0 ) gMode_ |= gUpdated_ ;
			locate(il,ic) ;
			getSecurity() ? outc(getSecurity(),l) : outs(s) ;
			isChange = 0 ;
		}

		if ( i < 0 || i >= l ) break ;

		locate(il,ic+i);
		c = get_key() ;
		if ( chinese() && is_chinese(c) ) {
			j = c ;
			c = get_key() ;
			if ( c & 0xff ) {
				c |= j << 8 ;
			}
		}

		if ( (c & 0x00ff) == 0 ) {	/* function keys */
			switch ( c ) {
			    case K_RIGHT :
				i++ ;
				if ( chinese() && is_chinese2(s,i) ) {
					i++ ;
					if ( (j=in_key()) != c ) unin_key(j) ;
				}
				if ( getConfirm() && i >= l ) i-- ;
				break ;

			    case K_LEFT :
				if ( --i > 0 &&
				     chinese() && is_chinese2(s,i) ) {
					i-- ;
					if ( (j=in_key()) != c ) unin_key(j) ;
				}
				if ( getConfirm() && i < 0 ) i = 0 ;
				break ;

			    case K_INS :
				isIns = isIns ^ gIsIns_ ;
				set_cursor(ctype,isIns?clow>>1:0,clow) ;
				break ;

			    case K_DEL :
				Delete(this,s,l,i);
				isChange = 1 ;
				break ;

			    case K_CLEAR :
				memset(&s[i],SP,l-i) ;
				isChange = 1 ;
				break ;

			    case K_HOME :
				i = (getDoSpecial() && i != 0) ? 0 : l ;
				break ;

			    case K_END :
				if ( getDoSpecial() && i+1 < l ) {
					j = i ;
					i = l ;
					while ( --i >= 0 && s[i] == SP ) ;
					if ( ++i >= l ) i-- ;
					if ( i == j ) i = l - 1 ;
					break ;
				}
				/* fall throught */

			    default : /* force exit loop */
				i = l ;
				break ;
			}
			atEnd = 0 ;
		} else {
			switch ( c ) {
			    case BS :
			    case DEL :
				if ( --i >= 0 ) {
					if ( chinese() && is_chinese2(s,i) ) {
						i-- ;
						if ( (j=in_key()) != c ) unin_key(j) ;
					}
					Delete(this,s,l,i);
					isChange = 1 ;
				}
				if ( getConfirm() && i < 0 ) i = 0 ;
				break ;

			    case HT  :
			    case ESC :
			    case LF  :
			    case CR  :	  /* return */
				i = l ;
				break ;

			    default :
				if ( getShiftLeft() &&
				     (atEnd || ((c&0xff00) && i+1==l)) ) {
					i += atEnd - Delete(this,s,l,0) ;
					if ( (c & 0xff00) && (l-i) < 2 )
						i -= Delete(this,s,l,0) ;
				}

				isChange = 1 ;
				i = isIns ? Insert(this,s,l,i,c) :
					    Replace(this,s,l,i,c) ;

				atEnd = 0 ;
				if ( i >= l &&
				     (getConfirm()||getShiftLeft()) ) {
					i = l - 1 ;
					atEnd = 1 ;
				}
				break ;
			}
		}
	}

	if ( getKeepIns() ) {
		if ( isIns )
			gMode_ |= gIsIns_ ;
		else
			gMode_ &= ~gIsIns_ ;
	}
	set_cursor(ct,ch,cl) ;

	return ((unsigned)c < ' ' || (c & 0xff) == 0) ? c : CR ;
}


int twin::get ( char s[], int pos )
{
	return get(0,0,s,pos) ;
}

