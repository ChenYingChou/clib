/* t2.cpp
*/

#include	<stdio.h>
#include	<stdlib.h>
#include	<ctype.h>
#include	<string.h>
#include	"twin.h"
#include	"scrlay.h"
#include	"inkey.h"
#include	"c0.h"

#define SCREEN_COMM1		250
#define FLD_CommPort		251
#define FLD_Irq 		252
#define FLD_IOport		253
#define FLD_Bps 		254
#define FLD_ConnectMode 	255
#define FLD_FlowControl 	256
#define FLD_DialCnt		257
#define FLD_AutoDetect		258

#define ShutUp(x)		if ( x ) ;

#define FC_NONE 		0
#define FC_XON_XOFF		1
#define FC_CTS_RTS		2
#define FC_BOTH 		(FC_XON_XOFF|FC_CTS_RTS)

#define S_MODEM 		0
#define S_DIRECT		1

/*****************************************************************************/
static	Byte	CommPort      = 1	;
static	Word	Bps	      = 9600	;
static	Byte	Irq	      = 5	;
static	Word	IOport	      = 0x1a0	;
static	Byte	FlowControl   = FC_NONE ;
/*****************************************************************************/

static int NEAR do_edit ( int scrNo, INPUT x[], int num )
{
	WINSCR	*w1		;

	pushWinScreen() ;
	w1 = getWinScreen(scrNo) ;
	readEdit(x,num) ;
	closeWinScreen(w1) ;
	popWinScreen(0) ;

	if ( lastKey != ESC ) {
		return 1 ;
	}
	return 0 ;
}

/*****************************************************************************/

static int CIrq1 ( int Pid, byte *src, ITEM px[], int numItem, int isEdit )
{
	int	i	;

	ShutUp(Pid) ;
	ShutUp(isEdit) ;
	for ( i = 0 ; i < numItem ; i++ ) {
		if ( Irq == atoi((char*)(px[i].str)) ) {
			*src = i ;
			break ;
		}
	}

	return TRUE ;
}


static int CIrq2 ( int Pid, byte *target, ITEM px[], int numItem )
{
	ShutUp(Pid) ;
	ShutUp(numItem) ;
	Irq = atoi((char*)(px[*target].str)) ;
	return TRUE ;
}


static int CIO1 ( int Pid, byte *src, ITEM px[], int numItem, int isEdit )
{
	ShutUp(Pid) ;
	ShutUp(px) ;
	ShutUp(numItem) ;
	ShutUp(isEdit) ;
	sprintf((char*)src,"%04X",IOport) ;
	return TRUE ;
}


static int CIO2 ( int Pid, byte *target, ITEM px[], int numItem )
{
	int	i	;
	word	x	;

	ShutUp(Pid) ;
	ShutUp(px) ;
	ShutUp(numItem) ;
	x = 0 ;
	for ( i = 0 ; i < 4 ; i++ ) {
		x <<= 4 ;
		if ( isdigit(target[i]) )
			x += target[i] - '0' ;
		else if ( target[i] >= 'A' && target[i] <= 'F' ||
			  target[i] >= 'a' && target[i] <= 'f'  )
			x += (target[i] & 0x07) + 9 ;
		else
			return FALSE ;
	}

	IOport = x ;
	return TRUE ;
}

/*****************************************************************************/
#define NUM_IO		2
static	Byte	_Irq			;
static	Byte	_IOport[5]		;
static	INPUT	_io[] = {
			{ FLD_Irq	   , CIrq1, CIrq2, &_Irq	    } ,
			{ FLD_IOport	   ,  CIO1,  CIO2,  _IOport	    }
		}	;
/*****************************************************************************/

static int CPort1 ( int Pid, byte *src, ITEM px[], int numItem, int isEdit )
{
	ShutUp(Pid) ;
	ShutUp(px) ;
	ShutUp(numItem) ;
	ShutUp(isEdit) ;
	*src = CommPort - 1 ;
	if ( *src >= 4 ) {
		doEdit(&_io[0],FALSE) ;
		doEdit(&_io[1],FALSE) ;
	}
	return TRUE ;
}


static int CPort2 ( int Pid, byte *target, ITEM px[], int numItem )
{
	ShutUp(Pid) ;
	ShutUp(px) ;
	ShutUp(numItem) ;
	CommPort = *target + 1 ;
	if ( *target >= 4 && lastKey == CR ) {
		readEdit(_io,NUM_IO) ;
	}
	return TRUE ;
}


static int CBps1 ( int Pid, byte *src, ITEM px[], int numItem, int isEdit )
{
	char	s[16]	;
	int	i	;

	ShutUp(Pid) ;
	ShutUp(isEdit) ;
	if ( Bps < 10 ) {
		i = numItem - 1 ;
	} else {
		sprintf(s,"%6u",Bps) ;
		for ( i = 0 ; i < numItem ; i++ ) {
			if ( strcmp(s,(char*)(px[i].str)) == 0 ) break ;
		}
		if ( i >= numItem ) i = numItem / 2 ;
	}
	*src = i ;
	return TRUE ;
}


static int CBps2 ( int Pid, byte *target, ITEM px[], int numItem )
{
	long	x	;

	ShutUp(numItem) ;
	if ( Pid != FLD_Bps ) return TRUE ;
	x = atol((char*)(px[*target].str)) ;
	Bps = x >= 65536 ? 1 : (word)x ;

	return TRUE ;
}


static int CFC1 ( int Pid, byte *src, ITEM px[], int numItem, int isEdit )
{
	ShutUp(Pid) ;
	ShutUp(px) ;
	ShutUp(numItem) ;
	ShutUp(isEdit) ;
	*src = (FlowControl == FC_NONE ? 0 : 1) ;
	return TRUE ;
}


static int CFC2 ( int Pid, byte *target, ITEM px[], int numItem )
{
	ShutUp(Pid) ;
	ShutUp(px) ;
	ShutUp(numItem) ;
	FlowControl = (*target == 0 ? FC_NONE : FC_CTS_RTS) ;
	return TRUE ;
}

/*****************************************************************************/
#define NUM_COMM1	6
static	Byte	_CommPort		;
static	Byte	_Bps			;
static	Byte	_ConnectMode  = S_MODEM ;
static	Byte	_FlowControl		;
static	Byte	_DialCnt      = 10	;
static	Byte	_AutoDetect   = 1	;
static	INPUT	_comm1[] = {
			{ FLD_CommPort	   ,CPort1,CPort2, &_CommPort	    } ,
			{ FLD_Bps	   , CBps1, CBps2, &_Bps	    } ,
			{ FLD_ConnectMode  ,	 0,	0, &_ConnectMode    } ,
			{ FLD_FlowControl  ,  CFC1,  CFC2, &_FlowControl    } ,
			{ FLD_DialCnt	   ,	 0,	0, &_DialCnt	    } ,
			{ FLD_AutoDetect   ,	 0,	0, &_AutoDetect     }
		}	;
/*****************************************************************************/

void set_comm1 ( )
{
	do_edit(SCREEN_COMM1,_comm1,NUM_COMM1) ;
}

int main ( )
{
	if ( addImage("t2.msg") == 0 ) return 1 ;

	set_comm1() ;
	return 0 ;
}

