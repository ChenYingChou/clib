/*
 * $Log: SCRIMG.CPP $
 * Revision 1.0  1996-02-06 23:29:12-0800  YCCHEN
 * Initial revision
 *
**/

#include	<stdio.h>
#include	<stdlib.h>
#include	<io.h>
#include	<fcntl.h>
#include	<malloc.h>
#include	<ctype.h>
#include	<dos.h>
#include	<stdarg.h>
#include	<string.h>
#include	"inkey.h"
#include	"scrlay.h"
#include	"c0.h"

/*****************************************************************************/
#if defined(__DJGPP__)
	#include <unistd.h>
	#define MALLOC			malloc
	#define FREE			free
	#define READ			_dos_read
	#define STRLEN			strlen
	#define MEMCPY			memcpy
#else
	#if defined(__WATCOMC__)
	    #if defined(__386__)
		#define MALLOC			malloc
		#define FREE			free
	    #else
		#define MALLOC			_fmalloc
		#define FREE			_ffree
	    #endif
	#else
		#define MALLOC			farmalloc
		#define FREE			farfree
	#endif

	#define READ			_dos_read
	#define STRLEN			_fstrlen
	#define MEMCPY			_fmemcpy
#endif
/*****************************************************************************/
GLOBAL	func	GetKeyFunc = (func)get_key	;
GLOBAL	int	lastKey 	;
GLOBAL	Byte	keepList	;
GLOBAL	Byte	colors[15] = { 0x01,0x09,0x70, 0x07,0x70, 0x01,0x78,
			       0x07,0x01,0x70, 0x07,0x07, 0x07,0x09,0xf0 } ;
/*****************************************************************************/
#define maxImage	8
static	Bfptr	image[maxImage] ;
static	int	numImage	;
static	Bfptr	curPtr		;
/*****************************************************************************/
#define maxKey		16
static	int	HotKey[maxKey] = { -1,-1,-1,-1, -1,-1,-1,-1,
				   -1,-1,-1,-1, -1,-1,-1,-1 } ;
static	func	HotFunc[maxKey] ;
/* 1 *************************************************************************/
static	int	nthEdit 	;
static	int	nRow		,
		nCol		;

int setNthEdit ( int nth )
{
	int	n = (nthEdit & 0x7fff) ;
	nthEdit = (nth >= 0 ? nth : n) ;
	return n ;
}


void setRowCol ( int row, int col )
{
	nRow = row ;
	nCol = col ;
}


void getRowCol ( int *row, int *col )
{
	*row = nRow ;
	*col = nCol ;
}

/*****************************************************************************/

static MESSAGE FAR * _findMessage ( Bfptr ptr, int ID )
{
	Word	offset	;
	MESSAGE FAR *p	;

	offset = *(Wfptr)ptr ;		/* data[0] == offset of MESSAGE */
	while ( offset ) {
		p = (MESSAGE FAR*)(ptr+offset) ;
		if ( p->ID == ID ) {
			curPtr = ptr ;
			return p ;
		}
		offset = p->nextPtr ;
	}
	return 0 ;
}


static SCREEN FAR * _findScreen ( Bfptr ptr, int ID )
{
	Word	offset	;
	SCREEN	FAR *p	;

	offset = *(Wfptr)(ptr+2) ;	/* data[2] == offset of SCREEN */
	while ( offset ) {
		p = (SCREEN FAR*)(ptr+offset) ;
		if ( p->ID == ID ) {
			curPtr = ptr ;
			return p ;
		}
		offset = p->nextPtr ;
	}
	return 0 ;
}


static PROMPT FAR * _findPrompt ( Bfptr ptr, int ID )
{
	Word	offset	;
	SCREEN	FAR *ps ;
	PROMPT	FAR *p	;

	offset = *(Wfptr)(ptr+2) ;	/* data[2] == offset of SCREEN */
	while ( offset ) {
		ps = (SCREEN FAR*)(ptr+offset) ;
		offset = ps->PTR ;
		while ( offset ) {
			p = (PROMPT FAR*)(ptr+offset) ;
			if ( p->ID == ID ) {
				curPtr = ptr ;
				return p ;
			}
			offset = p->nextPtr ;
		}
		offset = ps->nextPtr ;
	}
	return 0 ;
}


static char * _getMessage ( MESSAGE FAR *ptr )
{
	int	l = STRLEN((char FAR*)(ptr->string)) + 1 ;
	char	*p = (char*)malloc(l)	;

	if ( p ) MEMCPY(p,ptr->string,l) ;
	return p ;
}


static SCREEN * _getScreen ( SCREEN FAR *ptr )
{
	int	l = sizeof(SCREEN) - 1 ;
	char	*p = (char*)malloc(l)	;

	if ( p ) MEMCPY(p,ptr,l) ;
	return (SCREEN*)p ;
}


static PROMPT * _getPrompt ( PROMPT FAR *ptr )
{
	int	l = sizeof(PROMPT) + STRLEN((char FAR*)(ptr->string)) ;
	char	*p = (char*)malloc(l) ;

	if ( p ) MEMCPY(p,ptr,l) ;
	return (PROMPT*)p ;
}


static char * _getListPopUp ( Bfptr ptr, int sz, int num )
{
	Bfptr	r	;
	char	*p	;

	r = ptr + sz ;
	while ( num > 0 ) {
		r += STRLEN((char FAR*)(r+1)) + 2 ;	/* key:string[] */
		num-- ;
	}

	sz = r - ptr ;
	p = (char*)malloc(sz) ;
	if ( p ) MEMCPY(p,ptr,sz) ;
	return p ;
}


static LIST * _getList ( LIST FAR *ptr )
{
	return (LIST*)_getListPopUp((Bfptr)ptr,sizeof(LIST)-1,ptr->num) ;
}


static POPUP * _getPopUp ( POPUP FAR *ptr )
{
	return (POPUP*)_getListPopUp((Bfptr)ptr,sizeof(POPUP)-1,ptr->num) ;
}

/*****************************************************************************/

Word addImage ( char *filename )
{
	int	fd	;
	Word	size	;
	unsigned int l	;
	Bfptr	seg	;

	if ( filename == (char*)0 ) {
		while ( numImage > 0 ) {
			FREE(image[--numImage]) ;
			image[numImage] = 0 ;
		}
		return 0 ;
	}

	if ( numImage >= maxImage ) return 0 ;
	fd = open(filename,O_RDONLY|O_BINARY) ;
	if ( fd < 0 ) return 0 ;

#if defined(__WATCOMC__)
	_heapgrow() ;
#endif

	size = lseek(fd,0L,2) ;
	seg = (Bfptr)MALLOC(size) ;
	lseek(fd,0L,0) ;
	if ( seg != 0 ) {
		READ(fd,seg,size,&l) ;
		if ( l != size ) {
			FREE(seg) ;
			seg = 0 ;
		}
	}

	close(fd) ;
	if ( seg ) image[numImage++] = seg ;
	return numImage ;
}

/*****************************************************************************/

char * getMessage ( int messageID )
{
	int	i	;
	MESSAGE FAR *ptr;

	for ( i = 0 ; i < numImage ; i++ ) {
		if ( (ptr=_findMessage(image[i],messageID)) != 0 ) {
			return _getMessage(ptr) ;
		}
	}
	return 0 ;
}


MESSAGE FAR * findMessage ( int messageID )
{
	int	i	;
	MESSAGE FAR *ptr;

	for ( i = 0 ; i < numImage ; i++ ) {
		if ( (ptr=_findMessage(image[i],messageID)) != 0 ) {
			return ptr ;
		}
	}
	return 0 ;
}


SCREEN FAR * findScreen ( int screenID )
{
	int	i	;
	SCREEN FAR *ptr ;

	for ( i = 0 ; i < numImage ; i++ ) {
		if ( (ptr=_findScreen(image[i],screenID)) != 0 ) {
			return ptr ;
		}
	}
	return 0 ;
}


PROMPT FAR * findPrompt ( int messageID )
{
	int	i	;
	PROMPT FAR *ptr ;

	for ( i = 0 ; i < numImage ; i++ ) {
		if ( (ptr=_findPrompt(image[i],messageID)) != 0 ) {
			return ptr ;
		}
	}
	return 0 ;
}


int mprintf ( int messageID, ... )
{
	va_list ap	;
	char	s[200]	;
	char	*p	;
	int	id	;

	id = messageID < 0 ? -messageID : messageID ;
	p = getMessage(id) ;
	if ( p == (char*)0 ) {
		if ( messageID >= 0 ) xprintf("No such MESSAGE #%d",id) ;
		return 0 ;
	}

	va_start(ap,messageID) ;
	vsprintf(s,p,ap) ;
	outs(s) ;
	free(p) ;
	return 1 ;
}

/*****************************************************************************/

static	WINSCR	*_winscr_	;

static	twin	*_stack0[8]	;
static	WINSCR	*_stack1[8]	;
static	int	nthStock	;

int pushWinScreen ( void )
{
	if ( nthStock >= 8 ) return -1 ;  /* overflow */
	_stack1[nthStock] = _winscr_ ;
	if ( _winscr_ == (WINSCR*)0 ) _stack0[nthStock] = window_where() ;
	return ++nthStock ;
}


void popWinScreen ( int nth )
{
	if ( nth >= 0 && nthStock > 0 ) {
		if ( nth == 0 ) {
			nth = --nthStock ;
		} else {
			if ( nth == nthStock ) nthStock-- ;
			nth-- ;
		}
		useWinScreen(_stack1[nth]) ;
		if ( _winscr_ == (WINSCR*)0 ) (_stack0[nth])->use() ;
	}
}


WINSCR *useWinScreen ( WINSCR *w )
{
	WINSCR	*old = _winscr_ ;

	_winscr_ = w ;
	if ( w != (WINSCR*)0 ) {
		(w->win)->use() ;
		if ( (w->win)->is_color_mode() || w->colors[C_MONO] )
			memcpy(colors,w->colors,sizeof(colors)) ;
	}
	return old ;
}


void closeWinScreen ( WINSCR *w )
{
	if ( w != (WINSCR*)0 ) {
		(w->win)->restore_image() ;
		delete w->win ;
		free(w) ;
		if ( w == _winscr_ ) {
			_winscr_ = (WINSCR*)0 ;
			window_use(0) ;
		}
	}
}


WINSCR * getWinScreen ( int screenID )
{
	Word	seg, offset	;
	WINSCR	*ws		;
	SCREEN	FAR * fs	;
	int	r1, c1, r2, c2	;

	if ( (fs=findScreen(screenID)) == 0 ) return 0 ;

	ws = (WINSCR*)malloc(sizeof(WINSCR)) ;
	if ( ws == (WINSCR*)0 ) return 0 ;

	r1 = fs->row1 ;
	c1 = fs->col1 ;
	r2 = fs->row2 ;
	c2 = fs->col2 ;
	ws->win = new twin(r1,c1,r2,c2) ;
	if ( ws->win == 0 ) {
		free(ws) ;
		return 0 ;
	}

	if ( fs->status & STS_BACKUP ) (ws->win)->save_image() ;

	if ( (fs->status & STS_NO_IMAGE) == 0 )
		(ws->win)->restore_screen((Wfptr)(fs->screen)) ;

	MEMCPY(ws->colors,fs->colors,sizeof(ws->colors)) ;
	useWinScreen(ws) ;

	return ws ;
}

/*****************************************************************************/

ITEM * getItem ( Byte *s, int num )
{
	ITEM	*p, *q	;
	int	i	;

	if ( num <= 0 ) {
		Byte	*t = s	;
		num = 0 ;
		do {
			num++ ;
			while ( *++t != '\0' ) ;
		} while ( *++t != '\0' ) ;
	}

	p = (ITEM*)malloc(num*sizeof(ITEM)) ;
	if ( p != (ITEM*)0 ) {
		q = p ;
		for ( i = 0 ; i < num ; i++, q++ ) {
			q->key = *s++ ;
			q->str = s ;
			while ( *s++ != '\0' ) ;
		}
	}
	return p ;
}

/*****************************************************************************/

func setHKey ( int key, func x )
{
	int	i, j	;
	func	y	;

	if ( key == -1 ) {
		memset(HotKey,-1,sizeof(HotKey)) ;
	} else {
		j = -1 ;
		for ( i = maxKey ; --i >= 0 ; ) {
			if ( HotKey[i] == key ) {
				y = HotFunc[i] ;
				HotFunc[i] = x ;
				if ( x == (func)0 ) HotKey[i] = -1 ;
				return y ;
			}
			if ( HotKey[i] == -1 ) j = i ;
		}
		if ( j >= 0 ) {
			HotKey[j] = key ;
			HotFunc[j] = x ;
		}
	}
	return (func)0 ;
}


int getKey ( int nth )
{
	int	i, k	;

    getAgain:
	k = (GetKeyFunc)(nth) ;
	for ( i = 0 ; i < maxKey ; i++ ) {
		if ( k == HotKey[i] ) {
			k = (HotFunc[i])(k) ;
			if ( k == -1 ) goto getAgain ;
			break ;
		}
	}
	return k ;
}

/*****************************************************************************/

int _doList ( LIST *p, Byte *nth, ITEM X[], int num )
{
	int	ct, ch, cl	;
	int	row, col;
	int	i, j, k ;
	Byte	*pcol	;

	pcol = (Byte*)malloc(num) ;
	if ( pcol == (Byte*)0 ) return -1 ;

	get_cursor(&ct,&ch,&cl) ;
	set_cursor(1,0,0) ;

	SetColor(C_LIST) ;
/* 2 */ row = p->row + nRow ;
/* 2 */ col = p->col + nCol ;
	for ( i = 0 ; i < num ; i++ ) {
		locate(row,col) ;
		outs(X[i].str) ;
		pcol[i] = col ;
		col += strlen((char*)(X[i].str)) + p->skip ;
	}

	for ( i = *nth ; ; ) {
		if ( i < 0 )
			i = num - 1 ;
		else if ( i >= num )
			i = 0 ;

		SetColor(C_LIST_SELECT) ;
		locate(row,pcol[i]) ;
		outs(X[i].str) ;

		k = getKey(i) ;

		SetColor(C_LIST) ;
		locate(row,pcol[i]) ;
		outs(X[i].str) ;

		if ( k > ' ' && k < 128 ) {
			if ( islower(k) ) k = toupper(k) ;
			for ( j = i ; ; ) {
				if ( ++j >= num ) j = 0 ;
				if ( j == i ) break ;	/* not found */
				if ( k == X[j].key ) {
					i = j ;
					break ;
				}
			}
			continue ;
		}

		switch( k ) {
		    case K_LEFT :
			if ( (j=in_key()) != k ) unin_key(j) ;
			i-- ;
			break ;

		    case K_RIGHT :
			if ( (j=in_key()) != k ) unin_key(j) ;
		    case ' ' :
			i++ ;
			break ;

		    case ESC :
		    case K_UP :
		    case K_BTAB :
		    case K_DOWN :
		    case TAB :
		    case CR :
		    case F10 :
		    case K_PGDN :
		    case ('W'&0x1f) :
			set_cursor(ct,ch,cl) ;
			if ( k != ESC ) *nth = i ;
			if ( keepList > 1 ) {
				SetColor(C_LIST_SELECT) ;
				locate(row,pcol[i]) ;
				outs(X[i].str) ;
			} else if ( keepList == 0 ) {
				SetColor(C_BACKGROUND) ;
/* 2 */ 			locate(row,p->col+nCol) ;
/* 2 */ 			outc(' ',col-p->col-p->skip-nCol) ;
			}
			free(pcol) ;
			return k ;
		}
	}
}


int doList ( LIST *p, Byte *nth )
{
	ITEM	*px		;
	int	num		;
	int	sts		;

	num = p->num ;
	px = getItem(p->item,num) ;
	if ( px == (ITEM*)0 ) return -1 ;

	sts = _doList(p,nth,px,num) ;
	free(px) ;
	return sts ;
}

/*****************************************************************************/

static	POPUP	*pop		;
static	ITEM	*px		;
static	int	numItem 	;
static	int	pop_ptr 	;
static	int	skipCnt 	;
static	Byte	screenHigh	;
static	Byte	screenWidth	;

static int popup_init ( char *title )
{
	twin	*p	;

	if ( title ) ;
	p = window_where() ;
	set_wrap(0) ;
	SetColor(C_WINDOW_BORDER) ;
	window_use((twin*)0) ;

	switch( pop->frameType & 0x7f ) {
	    case FRAME_SINGLE :
		window_frame(p,pop->frameString) ;
		break ;
	    case FRAME_DOUBLE :
		window_cframe(p,(Word*)(pop->frameString)) ;
		break ;
	}

	window_use(p) ;
	SetColor(C_WINDOW_ITEM) ;
	clr_scr(1,25) ;

	screenHigh = p->display_high() ;
	screenWidth = p->display_width() ;
	if ( skipCnt >= screenHigh ) {
		pop_ptr = skipCnt - screenHigh + 1 ;
		skipCnt = screenHigh - 1 ;
	}

	return 1 ;	/* show data */
}


static void popup_move ( int nth )
{
	pop_ptr += nth ;
}


static int popup_get ( int nth )
{
	int	n	;

	n = pop_ptr + nth ;
	if ( n < 0 || n >= numItem ) return -1 ;
	return 0 ;
}


static void popup_show ( int current_line )
{
	int	nth	;

	nth = pop_ptr + current_line - 1 ;
	locate(current_line,1) ;
	xprintf("%-*.*s",screenWidth,screenWidth,px[nth].str) ;
}


static int popup_input ( int nth )
{
	int	n	;
	int	k	;

	if ( skipCnt < 0 ) {
		if ( pop_ptr + nth <= 0 )
			skipCnt = 0 ;
		else
			skipCnt++ ;
		return K_UP ;
	} else if ( skipCnt > 0 ) {
		if ( pop_ptr + nth >= numItem )
			skipCnt = 0 ;
		else
			skipCnt-- ;
		return K_DOWN ;
	}

	SetColor(C_WINDOW_SELECT) ;
	popup_show(nth+1) ;
	SetColor(C_WINDOW_ITEM) ;

	n = pop_ptr + nth ;
	switch( k=getKey(n) ) {
	    case SP :
	    case K_DOWN :
		if ( n + 1 < numItem ) return K_DOWN ;
	    case C_HOME :
		if ( pop_ptr > 0 ) {
			pop_ptr = 0 ;
			unin_key(K_HOME) ;
			return A_HOME ;
		}
		return K_HOME ;

	    case K_UP :
		if ( n > 0 ) return K_UP ;
	    case C_END :
		if ( pop_ptr + screenHigh < numItem ) {
			pop_ptr = numItem - screenHigh ;
			unin_key(K_END) ;
			return A_HOME ;
		}
		return K_END ;

	    case K_LEFT :
		if ( (n=in_key()) != k ) unin_key(n) ;
	    case K_BTAB :
		lastKey = K_UP ;
		return CR ;

	    case K_RIGHT :
		if ( (n=in_key()) != k ) unin_key(n) ;
	    case TAB :
		lastKey = K_DOWN ;
		return CR ;

	    case F10 :
	    case K_PGDN :
	    case ('W'&0x1f) :
		lastKey = K_PGDN ;
		return CR ;

	    case CR :
	    case ESC :
		lastKey = k ;
		return k ;

	    default:
		if ( k > ' ' && k < 128 ) {
			int	i, j	;
			if ( islower(k) ) k = toupper(k) ;
			for ( i = n  ; ; ) {
				if ( ++i >= numItem ) i = 0 ;
				if ( i == n ) break ;	/* not found */
				if ( k == px[i].key ) {
					j = i - n ;
					if ( (j<0?-j:j) < screenHigh ) {
						skipCnt = j ;
						return 0 ;
					}
					pop_ptr = i ;
					unin_key(K_HOME) ;
					return A_HOME ;
				}
			}
		}
		break ;
	}

	return k ;
}


int _doPopUp ( POPUP *p, Byte *nth, ITEM X[], int num )
{
static	int	input_list[] = { 0 }		;
	twin	*w0, *w ;
	int	k	;

	w0 = window_where() ;
	{
		int	r1, c1, r2, c2	;

/* 2 */ 	r1 = w0->row_based() + p->row1 - 1 + nRow ;
/* 2 */ 	c1 = w0->col_based() + p->col1 - 1 + nCol ;
/* 2 */ 	r2 = w0->row_based() + p->row2 - 1 + nRow ;
/* 2 */ 	c2 = w0->col_based() + p->col2 - 1 + nCol ;
		w = window_create(r1,c1,r2,c2) ;
	}
	if ( w == (twin*)0 ) return -1 ;

	window_use(w) ;
	window_save(w) ;
	set_cursor(1,0,0) ;

	pop = p ;
	px = X ;
	numItem = num ;
	skipCnt = *nth ;
	pop_ptr = 0 ;

	k = window_browse(w,1,popup_input,input_list,popup_init,
			  popup_get,popup_show,popup_move,0) ;
	if ( k > 0 ) {
		*nth = pop_ptr + k - 1 ;
	}

	window_restore(w) ;
	window_use(w0) ;
	window_close(w) ;

	return lastKey ;
}


int doPopUp ( POPUP *p, Byte *nth )
{
	ITEM	*px		;
	int	num		;
	int	k		;

	num = p->num ;
	px = getItem(p->item,num) ;
	if ( px == (ITEM*)0 ) return -1 ;

	k = _doPopUp(p,nth,px,num) ;
	free(px) ;
	return k ;
}

/*****************************************************************************/

static void NEAR Free3 ( void *p1, void *p2, void *p3 )
{
	if ( p1 != (void*)0 ) free(p1) ;
	if ( p2 != (void*)0 ) free(p2) ;
	if ( p3 != (void*)0 ) free(p3) ;
}


int doEdit ( INPUT *pi, int isEdit )
{
	PROMPT FAR *pf	;
	PROMPT	*p	;
	void	*pp	; /* point to LIST or POPUP */
	int	num	; /* number of ITEM px[] */
	ITEM	*px	;
	char	str[82] ;
	int	n	;
	int	isBin	;
/* 1 */ int	dRow, dCol, xRow, xCol ;

	if ( (pf=findPrompt(pi->promptID)) == 0 ) {
		/* error: no such PROMPT ID */
		return -1 ;
	}

	p = _getPrompt(pf) ;
	switch( p->dataType ) {
	    case TYPE_LIST :
		pp = (void*)_getList((LIST FAR*)(curPtr+pf->PTR)) ;
		num = ((LIST*)pp)->num ;
		px = getItem(((LIST*)pp)->item,num) ;
		break ;
	    case TYPE_POPUP :
		pp = (void*)_getPopUp((POPUP FAR*)(curPtr+pf->PTR)) ;
		num = ((POPUP*)pp)->num ;
		px = getItem(((POPUP*)pp)->item,num) ;
		break ;
	    default :
		num = 0 ;
		pp = (void*)0 ;
		px = (ITEM*)0 ;
		break ;
	}

/* 1 */ dRow = p->dataRow + nRow ;	xRow = p->row + nRow ;
/* 1 */ dCol = p->dataCol + nCol ;	xCol = p->col + nCol ;

	if ( !isEdit && dRow < 30 && p->dataWidth > 0 ) {
		SetColor(C_BACKGROUND) ;
		locate(dRow,dCol) ;
		outc(' ',p->dataWidth) ;
	}

	if ( pi->preValidation != 0 ) {
		n = (pi->preValidation)(pi->promptID,(Byte*)(pi->data),px,num,
					isEdit) ;
		if ( n <= 0 ) { /* skip or something error */
			Free3(px,pp,p) ;
			return n ;
		}
	}

	/* display prompt */
	SetColor(isEdit?C_MENU_SELECT:C_MENU) ;
	locate(xRow,xCol) ;
	outs(p->string) ;

	/* display data */
	SetColor(C_DATA) ;
	locate(dRow,dCol) ;
	switch( p->dataType ) {
	    case TYPE_BIT8 :
		sprintf(str,"%*u",p->dataWidth,*(Byte*)pi->data) ;
		outs(str) ;
		break ;
	    case TYPE_BIT8S :
		sprintf(str,"%*d",p->dataWidth,*(char*)pi->data) ;
		outs(str) ;
		break ;
	    case TYPE_BIT16 :
		sprintf(str,"%*u",p->dataWidth,*(Word*)pi->data) ;
		outs(str) ;
		break ;
	    case TYPE_BIT16S :
		sprintf(str,"%*d",p->dataWidth,*(short*)pi->data) ;
		outs(str) ;
		break ;
	    case TYPE_BIT32 :
		sprintf(str,"%*lu",p->dataWidth,*(unsigned long*)pi->data) ;
		outs(str) ;
		break ;
	    case TYPE_BIT32S :
		sprintf(str,"%*ld",p->dataWidth,*(long*)pi->data) ;
		outs(str) ;
		break ;
	    case TYPE_STRING :
		n = strlen((char*)(pi->data)) ;
		if ( p->dataWidth > 0 ) {
			outc(' ',p->dataWidth) ;
			if ( p->dataWidth < n ) n = p->dataWidth ;
		}
		memcpy(str,pi->data,n) ;
		str[n] = '\0' ;
		outs(str) ;
		break ;
	    case TYPE_LIST :
	    case TYPE_POPUP :
		n = *(Byte*)(pi->data) ;
		if ( n >= num ) n = 0 ;
		if ( dRow <= 25 ) xprintf("%-*s",p->dataWidth,px[n].str) ;
		*(Byte*)str = n ;
		break ;
	}

	if ( !isEdit ) {
		Free3(px,pp,p) ;
		return 0 ;	/* display only */
	}

	isBin = (p->dataType >= TYPE_BIN_LOW && p->dataType <= TYPE_BIN_HIGH) ;
	if ( isBin ) strcpy(&str[32],str) ;

	do {
		switch( p->dataType ) {
		    case TYPE_LIST :
			lastKey = _doList((LIST*)pp,(Byte*)str,px,num) ;
			break ;

		    case TYPE_POPUP :
			lastKey = _doPopUp((POPUP*)pp,(Byte*)str,px,num) ;
			break ;

		    default :
			{	twin	*w	;
				Byte	gMode	;
				int	pos = 0	;
				w = window_where() ;
				gMode = w->getMode() & ~gIsIns_ ;
				if ( isBin && w->getBinActive() ) {
					w->set_getShiftLeft(1) ;
					w->set_getSkipBlank(1) ;
				}
				if ( lastKey == K_LEFT && (gMode & gConfirm_) == 0 )
					pos = strlen(str) - 1 ;
				SetColor(C_INPUT) ;
				lastKey = get_string0(dRow,dCol,str,pos) ;
				w->set_getMode(gMode|(w->getMode()&gIsIns_)) ;
			}
			break ;
		}
	} while ( lastKey != ESC &&
		  pi->postValidation != 0 &&
		  (pi->postValidation)(pi->promptID,(Byte*)str,px,num) <= 0 ) ;

	if ( lastKey != ESC ) { /* save data */
		switch( p->dataType ) {
		    case TYPE_STRING :
			memcpy(pi->data,str,strlen(str)) ;
			break ;
		    case TYPE_LIST :
		    case TYPE_POPUP :
			*(Byte*)pi->data = *(Byte*)str ;
			break ;
		    case TYPE_BIT8 :
		    case TYPE_BIT8S :
			*(Byte*)pi->data = atoi(str) ;
			break ;
		    case TYPE_BIT16 :
		    case TYPE_BIT16S :
			*(Word*)pi->data = atoi(str) ;
			break ;
		    case TYPE_BIT32 :
		    case TYPE_BIT32S :
			*(long*)pi->data = atol(str) ;
			break ;
		}
	} else {	/* restore original value */
		if ( isBin )	/* restore ascii string of BIN */
			strcpy(str,&str[32]) ;
		else
			memcpy(str,pi->data,strlen(str)) ;
	}

	/* display prompt */
	SetColor(C_MENU) ;
	locate(xRow,xCol) ;
	outs(p->string) ;

	SetColor(C_DATA) ;
	locate(dRow,dCol) ;
	if ( isBin || p->dataType == TYPE_STRING )
		outs(str) ;
	else if ( dRow > 0 && dRow < 30 )
		xprintf("%-*s",p->dataWidth,px[*(Byte*)pi->data].str) ;

	Free3(px,pp,p) ;
	return 1 ;
}

/*****************************************************************************/

int showEdit ( INPUT *pi, int color )
{
	Byte	oldData = colors[C_DATA];
	int	sts			;

	colors[C_DATA] = ( color < 0 ? -color : colors[color] ) ;
	sts = doEdit(pi,FALSE) ;
	colors[C_DATA] = oldData ;
	return sts ;
}

/*****************************************************************************/

int readEdit ( INPUT inp[], int num )
{
	int	i, dir	;
	int	sts	;

	for ( i = 0 ; i < num ; i++ )
		doEdit(&inp[i],0) ;

	dir = 1 ;
/* 1 */ i = nthEdit < 0 ? 0 : nthEdit ;
	do {
		if ( i < 0 )
			i = num - 1 ;
		else if ( i >= num )
			i = 0 ;
		nthEdit = i | 0x8000 ;

		sts = doEdit(&inp[i],1) ;
		if ( sts <= 0 ) {	/* skip */
			i += dir ;
/* ..			lastKey = CR ;	/**/
		} else {
			switch( lastKey ) {
			    case ESC :
				return 0 ;

			    case C_HOME :
				i = 0 ;
				break ;

			    case K_UP :
			    case K_LEFT :
			    case K_BTAB :
				dir = -1 ;
				i-- ;
				break ;

			    case K_DOWN :
			    case K_RIGHT :
			    case TAB :
			    case CR :
				dir = 1 ;
				i++ ;
				break ;

			    case K_PGDN :
			    case ('W'&0x1f) :
			    case F10 :
				return 1 ;
			}
		}

	} while ( lastKey != CR || i < num ) ;

	return 1 ;
}

/*****************************************************************************/
