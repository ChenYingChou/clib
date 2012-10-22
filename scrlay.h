/*
 * $Log: SCRLAY.H $
 * Revision 1.0  1996-02-06 23:29:14-0800  YCCHEN
 * Initial revision
 *
**/

#include	"typedef.h"

#ifndef _SCRLAY_
#define _SCRLAY_	1

/* compatiable with old version */
#define word		Word
#define byte		Byte
#define dword		Dword

#define PTR_SIZE	2
#define MSG_FIRST	0
#define SCR_FIRST	(MSG_FIRST+PTR_SIZE)

#define C_DATA		0
#define C_INPUT 	1
#define C_MESSAGE	2
#define C_MENU		3
#define C_MENU_SELECT	4
#define C_LIST		5
#define C_LIST_SELECT	6
#define C_WINDOW_BORDER 7
#define C_WINDOW_ITEM	8
#define C_WINDOW_SELECT 9
#define C_TEXT		10
#define C_BACKGROUND	11
#define C_RESERVED	12
#define C_MARK		13
#define C_MONO		14

#pragma pack(1)

typedef struct	{
	Word	nextPtr 	;
	short	ID		;
	Byte	string[1]	; /* varying: ASCIZ */
}	MESSAGE ;


typedef struct	{
	Word	nextPtr 	;
	short	ID		;
	Byte	row1		,
		col1		,
		row2		,
		col2		;
	Byte	status		;
	Byte	colors[15]	;
	Word	PTR		; /* PROMPT pointer */
	Byte	screen[1]	; /* varying: raw screen */
}	SCREEN	;

#define STS_BACKUP	0x01
#define STS_NO_IMAGE	0x02


typedef struct	{
	Word	nextPtr 	;
	short	ID		;
	Byte	row		,
		col		;
	Byte	dataRow 	,
		dataCol 	,
		dataWidth	,
		dataType	;
	Word	PTR		; /* LIST/POPUP pointer */
	Byte	string[1]	; /* varying: ASCIZ */
}	PROMPT	;

#define TYPE_STRING	1
#define TYPE_LIST	2
#define TYPE_POPUP	3
#define TYPE_BIT8	4
#define TYPE_BIT8S	5
#define TYPE_BIT16	6
#define TYPE_BIT16S	7
#define TYPE_BIT32	8
#define TYPE_BIT32S	9
#define TYPE_BIN_LOW	TYPE_BIT8
#define TYPE_BIN_HIGH	TYPE_BIT32S


typedef struct	{
	Byte	num		;
	Byte	row		,
		col		,
		skip		;
	Byte	item[1] 	; /* varying: key+ASCIZ ... */
}	LIST	;


typedef struct	{
	Byte	num		;
	Byte	row1		,
		col1		,
		row2		,
		col2		;
	Byte	frameType	; /* 0,1,2 */
	Byte	frameString[16] ;
	Byte	item[1] 	; /* varying: key+ASCIZ ... */
}	POPUP	;

#define FRAME_NONE	0
#define FRAME_SINGLE	1
#define FRAME_DOUBLE	2

/*****************************************************************************/

typedef struct	{
	Byte	key	;
	Byte	*str	;
}	ITEM	;


#ifndef _FUNC_
#define _FUNC_		1
typedef int	(*func)( int )	;
typedef int	(*funcV1)( int Pid, byte *x, ITEM px[], int numItem, int isEdit )  ;
typedef int	(*funcV)( int Pid, byte *x, ITEM px[], int numItem )  ;
#endif

typedef struct {
	short	promptID	;
	funcV1	preValidation	;
	funcV	postValidation	;
	void	*data		; /* STRING:ASCIIZ, LIST/POPUP:(int*) */
}	INPUT	;

#pragma pack()
/*---------------------------------------------------------------------------*/
#include	"twin.h"

#pragma pack(1)

typedef struct {
	twin	*win		;
	Byte	colors[15]	;
}	WINSCR	;

#pragma pack()
/*---------------------------------------------------------------------------*/
#ifdef __cplusplus
extern "C" {
#endif

extern	func	GetKeyFunc	;
extern	int	lastKey 	;
extern	Byte	keepList	;
extern	Byte	colors[15]	;
/* scrimg.c -----------------------------------------------------------------*/
#define SetColor(c)		set_attr(colors[c])

Word	addImage( char *file )			;

char	* getMessage( int ID )			;
int	mprintf( int ID, ... )			;

ITEM	* getItem( char *s, int num )		;

int	pushWinScreen( void )			;
void	popWinScreen( int nth ) 		;
WINSCR	* getWinScreen( int ID )		;
WINSCR	* useWinScreen( WINSCR *ws )		;
void	closeWinScreen( WINSCR *ws )		;

func	setHKey( int key, func x )		;
int	getKey( )				;
int	showEdit( INPUT *pi, int color )	;
int	readEdit( INPUT input[], int num )	;
int	doEdit( INPUT *pi, int isEdit ) 	;
int	setNthEdit( int nth )			;
void	setRowCol( int row, int col )		;
void	getRowCol( int *row, int *col ) 	;

int	doList( LIST *pl, Byte *nth )		;
int	doPopUp( POPUP *pp, Byte *nth ) 	;

MESSAGE FAR * findMessage( int ID )		;
SCREEN	FAR * findScreen( int ID )		;
PROMPT	FAR * findPrompt( int ID )		;

#ifdef __cplusplus
};
#endif
/*****************************************************************************/

#endif	/* _SCR_LAY_ */

