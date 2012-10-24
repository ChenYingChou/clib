/* commdrv.c
*/

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "pchw.h"

#define _V_	/* don't care volatile to QUEUE, so compiler do optimize */
#include "comm.h"

#ifndef DEF_INP_SIZE
#define DEF_INP_SIZE		2048
#endif

#ifndef DEF_OUT_SIZE
#define DEF_OUT_SIZE		1024
#endif

#define nop()			/**/
#define LOCAL			static
#undef	LOCKED_CODE
#define LOCKED_CODE		/**/

#define XON			0x11
#define XOFF			0x13

/* UART registers # */
#define TBR			0
#define RBR			0
#define DLL			0
#define DLM			1
#define IER			1
#define IIR			2
#define LCR			3
#define MCR			4
#define LSR			5
#define MSR			6
#define FCR			2

/* LSR: Line Status Register */
#define RxReady 		0x01
#define DataOverRun		0x02
#define ParityError		0x04
#define FrameError		0x08
#define BreakSignal		0x10
#define TxReady 		0x20
#define TxShiftReady		0x40

/* LCR: Line Control Register */
#define Data5			0x00
#define Data6			0x01
#define Data7			0x02
#define Data8			0x03
#define Stop1			0x00
#define Stop2			0x04
#define ParityNone		0x00
#define ParityEven		0x18
#define ParityOdd		0x08
#define ParityMark		0x28
#define ParitySpace		0x38
#define BreakActive		0x40
#define DivLatch		0x80

/* MCR: Modem Control Register */
#if !defined(DTR)
#define DTR			0x01
#define RTS			0x02
#endif
#define OUT1			0x04
#define OUT2			0x08
#define LOOP			0x10

/* MSR: Modem Status Register */
#if !defined(dCTS)
#define dCTS			0x01
#define dDSR			0x02
#define dRING			0x04
#define dCD			0x08
#define CTS			0x10
#define DSR			0x20
#define RING			0x40
#define CD			0x80
#endif

#define S_init_send		0x01
#define S_init_xon		0x02
#define S_xmt_soft_off		0x04
#define S_xmt_hard_off		0x08
#define S_rcv_soft_off		0x10
#define S_rcv_hard_off		0x20
#define S_xmt_wait		0x40
#define S_xmt_pending		0x80

/* Flow Control */
#if !defined(FC_XON_XOFF)
#define FC_XON_XOFF		0x01
#define FC_CTS_RTS		0x02
#define FC_NO_XMT_XON_XOFF	0x04		/* for HP only	*/
#endif

#define SetBit(x,bit)		(x|=(bit))
#define ClrBit(x,bit)		(x&=~(bit))
#define GetBit(x,bit)		(x&(bit))

/*****************************************************************************/
static	Byte	iobuf1[DEF_INP_SIZE+DEF_OUT_SIZE] LOCKED_DATA ;
static	Byte	iobuf2[DEF_INP_SIZE+DEF_OUT_SIZE] LOCKED_DATA ;
static	COMM	com1 LOCKED_DATA
		= { 0x3f8,4,0,19200,0,
		    {iobuf1,DEF_INP_SIZE},
		    {iobuf1+DEF_INP_SIZE,DEF_OUT_SIZE}
		  } ;
static	COMM	com2 LOCKED_DATA
		= { 0x2f8,3,0,19200,0,
		    {iobuf2,DEF_INP_SIZE},
		    {iobuf2+DEF_INP_SIZE,DEF_OUT_SIZE}
		  } ;
/*****************************************************************************/
LOCAL void _dummy ( void ) { }
/*****************************************************************************/

#define queue_length(x) 	((x)->filled)
#define is_queue_full(x)	(queue_length(x) >= (x)->size)

LOCAL int queue_get ( QUEUE *p ) LOCKED_CODE ;
LOCAL int queue_get ( QUEUE *p )
{
	int	ch = *(p->ptr+p->rptr) ;
	p->filled-- ;
	if ( ++(p->rptr) >= p->size ) p->rptr = 0 ;
	return ch ;
}


LOCAL void queue_put ( QUEUE *p, Byte ch ) LOCKED_CODE ;
LOCAL void queue_put ( QUEUE *p, Byte ch )
{
	*(p->ptr+p->wptr) = ch ;
	p->filled++ ;
	if ( ++(p->wptr) >= p->size ) p->wptr = 0 ;
}

/*****************************************************************************/

LOCAL void xmt_int ( COMM *sio ) LOCKED_CODE ;
LOCAL void xmt_int ( COMM *sio )
{
	int	cnt	;

	sio->force_xmt_ready = 0 ;
	if ( GetBit(sio->sts,S_init_send) ) {
		ClrBit(sio->sts,S_init_send) ;
		SetBit(sio->sts,S_xmt_pending) ;
		outportb(sio->base+TBR,GetBit(sio->sts,S_init_xon)?XON:XOFF) ;
		sio->cLSR |= TxReady ;
		return ;
	}

	ClrBit(sio->sts,S_xmt_pending) ;

	cnt = queue_length(&sio->outbuf) ;
	cnt > 0 ? SetBit(sio->sts,S_xmt_wait) : ClrBit(sio->sts,S_xmt_wait) ;
	if ( GetBit(sio->sts,S_xmt_wait) == 0 ||
	     GetBit(sio->sts,S_xmt_hard_off|S_xmt_soft_off) )
		return ;

	ClrBit(sio->sts,S_xmt_wait) ;
	SetBit(sio->sts,S_xmt_pending) ;

	if ( sio->fifo_size > 0 ) {
		if ( cnt > sio->fifo_size ) cnt = sio->fifo_size ;
	} else {
		cnt = 1 ;
	}

	do {
		outportb(sio->base+TBR,queue_get(&sio->outbuf)) ;
	} while ( --cnt > 0 ) ;

	sio->cLSR |= TxReady ;
}


LOCAL void modem_status_int ( COMM *sio ) LOCKED_CODE ;
LOCAL void modem_status_int ( COMM *sio )
{
	sio->cMSR = inportb(sio->base+MSR) ;
	if ( GetBit(sio->flow,FC_CTS_RTS) ) {	/* CTS/RTS flow control */
		if ( GetBit(sio->cMSR,CTS) ) {
			ClrBit(sio->sts,S_xmt_hard_off) ;
			if ( GetBit(sio->sts,S_xmt_wait) ) xmt_int(sio) ;
		} else {
			SetBit(sio->sts,S_xmt_hard_off) ;
		}
	}
}


LOCAL void rcv_int ( COMM *sio ) LOCKED_CODE ;
LOCAL void rcv_int ( COMM *sio )
{
	Byte	ch, sts ;

	while ( (sts=inportb(sio->base+LSR)) & RxReady ) {
		if ( sts & TxReady ) sio->force_xmt_ready = 1 ;
		nop() ;
		ch = inportb(sio->base+RBR) ;

		if ( GetBit(sio->flow,FC_XON_XOFF) ) {
			if ( GetBit(sio->flow,FC_NO_XMT_XON_XOFF) == 0 ) {
				if ( ch == XON ) {
					ClrBit(sio->sts,S_xmt_soft_off) ;
					if ( GetBit(sio->sts,S_xmt_wait) )
						xmt_int(sio) ;
					continue ;
				}
				if ( ch == XOFF ) {
					SetBit(sio->sts,S_xmt_soft_off) ;
					continue ;
				}
			}
			if ( GetBit(sio->sts,S_rcv_soft_off) == 0 &&
			     queue_length(&sio->inpbuf) >= sio->high_level ) {
				SetBit(sio->sts,S_rcv_soft_off|S_init_send) ;
				ClrBit(sio->sts,S_init_xon) ;
				if ( GetBit(sio->sts,S_xmt_pending) == 0 )
					xmt_int(sio) ;
			}
		}

		if ( GetBit(sio->flow,FC_CTS_RTS) &&
		     GetBit(sio->sts,S_rcv_hard_off) == 0 &&
		     queue_length(&sio->inpbuf) >= sio->high_level ) {
			SetBit(sio->sts,S_rcv_hard_off) ;
			sio->cMCR &= ~RTS ;
			outportb(sio->base+MCR,sio->cMCR) ;
		}

		sio->cLSR |= sts & (FrameError|ParityError|DataOverRun|
				    BreakSignal|RxReady) ;

		if ( is_queue_full(&sio->inpbuf) )
			sio->cLSR |= DataOverRun ;
		else
			queue_put(&sio->inpbuf,ch) ;
	}
}


LOCAL void sio_int ( COMM *sio ) LOCKED_CODE ;
LOCAL void sio_int ( COMM *sio )
{
	Byte	id	;
	int	tFlag=0 ;

	sio->force_xmt_ready = 0 ;
	while ( ((id=inportb(sio->base+IIR)) & 1) == 0 ) {
		switch( id & 0x07 ) {
		    case 0 :
			modem_status_int(sio) ;
			tFlag |= TASK_MS ;
			break ;
		    case 2 :
			xmt_int(sio) ;
			tFlag |= TASK_TX ;
			break ;
		    case 4 :
			rcv_int(sio) ;
			tFlag |= TASK_RX ;
			break ;
#if 0	/* not active */
		    case 6 : /* received errors & break Signal */
			sio->cLSR |= inportb(sio->base+LSR) & (FrameError|
					ParityError|DataOverRun|BreakSignal) ;
			tFlag |= TASK_LS ;
			break ;
#endif
		}
	}

	if ( sio->force_xmt_ready ) {
		xmt_int(sio) ;
		tFlag |= TASK_TX ;
	}

	if ( sio->taskFlag ) *(sio->taskFlag) |= tFlag ;
}


LOCAL int sio_int1 ( void ) LOCKED_CODE ;
LOCAL int sio_int1 ( void )
{
	sio_int(&com1) ;
	return 0 ;
}


LOCAL int sio_int2 ( void ) LOCKED_CODE ;
LOCAL int sio_int2 ( void )
{
	sio_int(&com2) ;
	return 0 ;
}

/*****************************************************************************/
LOCAL END_OF_FUNCTION(_dummy);
/*****************************************************************************/

LOCAL void set_bps ( COMM *sio )
{
	int	i	;

	i = 115200LU / (sio->bps > 0 ? sio->bps : 9600) ;
	sio->bps = 115200LU / i ;

	if ( sio->mbps == 0 || GetBit(inportb(sio->base+MSR),CD) == 0 )
		sio->mbps = sio->bps ;

	ENTER
	outportb(sio->base+LCR,sio->cLCR|DivLatch) ;
	nop() ;
	outportb(sio->base+DLL,i) ;
	nop() ;
	outportb(sio->base+DLM,i>>8) ;
	nop() ;
	outportb(sio->base+LCR,sio->cLCR) ;
	EXIT
}


LOCAL void change_fifosize ( COMM *sio, int TxSize, int RxSize )
{
	int	x	;

	if ( sio->cFCR & 1 ) {
		sio->fifo_size = TxSize ;
		switch( RxSize ) {
		    case 0 : /* disable FIFO */
			x = 0x00 ;
			sio->fifo_size = 0 ;
			break ;
		    case 1 :
			x = 0x01 ;
			break ;
		    case 4 :
			x = 0x41 ;
			break ;
		    case 8 :
			x = 0x81 ;
			break ;
		    default :
			x = 0xc1 ;
			break ;
		}
		outportb(sio->base+FCR,x) ;
		sio->cFCR = x | 1 ;
	} else {
		sio->fifo_size = 0 ;
	}
}


LOCAL int install_port ( COMM *sio )
{
static	int	initialize = 1 ;
	int	base	;
	int	isFirst ;
	int	inta, i ;

	if ( initialize ) {
		initialize = 0 ;
		LOCK_FUNCTION(_dummy) ;
	}

	if ( sio == NULL ) return -1 ;
	base = sio->base ;
	isFirst = (sio->irq_bit == 0) ;

	if ( isFirst ) {
		sio->cMCR = RTS | DTR ;
		if ( base & 0x0800 ) {	/* enable OUT1 */
			sio->cMCR |= OUT1 ;
			base &= 0x03ff ;
			sio->base = base ;
		}

		sio->xMCR = inportb(base+MCR) ;
		if ( sio->xMCR & 0xe0 ) return -1 ;
		nop() ;
		outportb(base+MCR,sio->xMCR|0xe0) ;
		nop() ;
		if ( inportb(base+MCR) != sio->xMCR ) return -1 ;

		sio->sts     = 0 ;
		sio->irq_bit = 1 << (sio->irq&7) ;
		sio->flow    = 0 ;

		sio->xLCR = inportb(base+LCR) & 0x7f ;
		nop() ;

		sio->xIER = inportb(base+IER) ;
		nop() ;

		ENTER
		outportb(base+LCR,sio->cLCR|DivLatch) ;
		nop() ;
		sio->xDLL = inportb(base+DLL) ;
		nop() ;
		sio->xDLM = inportb(base+DLM) ;
		nop() ;
		outportb(base+LCR,sio->cLCR) ;
		nop() ;
		sio->inpbuf.filled = sio->inpbuf.wptr = sio->inpbuf.rptr = 0 ;
		sio->outbuf.filled = sio->outbuf.wptr = sio->outbuf.rptr = 0 ;
		EXIT

		/* check 1655x */
		outportb(base+FCR,0xc7) ;
		nop() ;
		if ( (inportb(base+FCR) & 0xc0) == 0xc0 ) {
			sio->cFCR = 1 ;
			change_fifosize(sio,3,1) ;
		} else {
			sio->cFCR = 0 ;
			outportb(base+FCR,0) ;
			sio->fifo_size = 0 ;
		}

		outportb(base+IER,0) ;
		if ( sio->taskComm ) (sio->taskComm)(sio,TASK_INIT,0) ;
/****/		_install_irq(sio->irq,sio==&com1?sio_int1:sio_int2) ;
		atexit(comm_close_all) ;
	}

	if ( sio->cFCR & 1 ) {
		outportb(base+FCR,sio->cFCR|0x06) ;
		nop() ;
	}

	if ( sio->bps == 0 )		/* use original bps */
		sio->bps = 115200L / ( (int)(sio->xDLM) * 256 + sio->xDLL ) ;
	else
		set_bps(sio) ;

	sio->cMCR |= OUT2 ;
	outportb(base+MCR,sio->cMCR) ;
	nop() ;

	while ( inportb(base+LSR) & RxReady ) {
		nop() ;
		inportb(base+RBR) ;
		nop() ;
	}

	nop() ;
	inportb(base+LSR) ;
	nop() ;
	sio->cMSR = inportb(base+MSR) ;
	nop() ;
	inportb(base+IIR) ;
	nop() ;

	if ( sio->irq > 7 ) {
		i = inportb(0x21) & ~ 0x04 ;
		nop() ;
		outportb(0x21,i) ;
		inta = 0xa1 ;
	} else
		inta = 0x21 ;

	i = inportb(inta) ;
	if ( isFirst ) sio->xIRQ = i & sio->irq_bit ;
	nop() ;
	outportb(inta,i&~sio->irq_bit) ;

	/* Enable interrupts with correction for posibble loss of the
	   first transmit interrupt on 8250 and 8250B chips */
	while ( (inportb(base+LSR) & TxReady) == 0 )
		nop() ;
	nop() ;

	ENTER
	outportb(base+IER,0x0b) ;
	nop() ;
	outportb(base+IER,0x0b) ;
	EXIT

	return 0 ;
}


/*LOCAL int remove_port ( COMM *sio, int restore )*/
int comm_close ( COMM *sio, int restore )
{
	int	base		;
	int	inta, i 	;

	if ( sio == NULL || sio->irq_bit == 0 ) return -1 ;

	base = sio->base ;

	ENTER
	outportb(base+IER,0) ;

/****/	_remove_irq(sio->irq) ;

	if ( restore ) {
		outportb(base+LCR,sio->xLCR|DivLatch) ;
		nop() ;
		outportb(base+DLL,sio->xDLL) ;
		nop() ;
		outportb(base+DLM,sio->xDLM) ;
		nop() ;
		outportb(base+LCR,sio->xLCR) ;
		nop() ;
		outportb(base+MCR,sio->xMCR) ;
		nop() ;
	} else {
		outportb(base+MCR,(sio->xMCR&(OUT1|OUT2|LOOP))|
			      (sio->cMCR&(DTR|RTS))) ;
		nop() ;
	}

	outportb(base+FCR,0) ;
	outportb(base+IER,sio->xIER) ;
	nop() ;

	inta = sio->irq > 7 ? 0xa1 : 0x21 ;
	i = inportb(inta) & ~sio->irq_bit ;
	nop() ;
	outportb(inta,i|sio->xIRQ) ;
	EXIT

	sio->irq_bit = 0 ;
	if ( sio->taskComm ) (sio->taskComm)(sio,TASK_FINISH,0) ;
	return 0 ;
}


/*LOCAL int put_port ( COMM *sio, Byte ch )*/
int comm_putc ( COMM *sio, int ch )
{
	if ( is_queue_full(&sio->outbuf) ) return 0 ;

	ENTER
	queue_put(&sio->outbuf,ch) ;
	if ( GetBit(sio->sts,S_xmt_pending) == 0 ) xmt_int(sio) ;
	EXIT
	return 1 ;
}


LOCAL int get_port ( COMM *sio )
{
	int	ch	;

	if ( queue_length(&sio->inpbuf) == 0 ) return -1 ;

	ENTER
	ch = queue_get(&sio->inpbuf) ;
	if ( GetBit(sio->flow,FC_XON_XOFF|FC_CTS_RTS) &&
	     queue_length(&sio->inpbuf) <= sio->low_level ) {
		if ( GetBit(sio->sts,S_rcv_soft_off) ) {
			ClrBit(sio->sts,S_rcv_soft_off) ;
			SetBit(sio->sts,S_init_send|S_init_xon) ;
			if ( GetBit(sio->sts,S_xmt_pending) == 0 )
				xmt_int(sio) ;
		}

		if ( GetBit(sio->sts,S_rcv_hard_off) ) {
			ClrBit(sio->sts,S_rcv_hard_off) ;
			sio->cMCR |= RTS ;
			outportb(sio->base+MCR,sio->cMCR) ;
		}
	}
	EXIT

	return ch ;
}


LOCAL int change_protocol ( COMM *sio, Byte newFC )
{
	Byte	oldFC, stsFC	;

	oldFC = sio->flow ;
	stsFC = oldFC ^ newFC ;
	sio->flow = newFC ;

	if ( newFC ) {	/* need flow control */
		if ( sio->high_level == 0 || sio->high_level >= sio->inpbuf.size )
			sio->high_level = sio->inpbuf.size / 8 * 7 ;
		if ( sio->low_level == 0 || sio->low_level >= sio->high_level )
			sio->low_level = sio->inpbuf.size / 2 ;
	}

	if ( GetBit(stsFC,FC_CTS_RTS) ) {	 /* change CTS/RTS */
		ClrBit(sio->sts,S_rcv_hard_off) ;
		outportb(sio->base+MCR,sio->cMCR|=RTS) ;
		nop() ;

		sio->cMSR = inportb(sio->base+MSR) ;
		if ( GetBit(oldFC,FC_CTS_RTS) || GetBit(sio->cMSR,CTS) )
			ClrBit(sio->sts,S_xmt_hard_off) ;
		else
			SetBit(sio->sts,S_xmt_hard_off) ;
	}

	if ( GetBit(stsFC,FC_XON_XOFF) ) {	/* change XonXoff */
		if ( GetBit(sio->sts,S_rcv_soft_off) ) {
			SetBit(sio->sts,S_init_send|S_init_xon) ;
		}
	}
	ClrBit(sio->sts,S_xmt_soft_off|S_rcv_soft_off) ;

	ENTER
	if ( GetBit(sio->sts,S_xmt_pending) == 0 ) xmt_int(sio) ;
	EXIT

	return oldFC ;
}

/*****************************************************************************/

COMM * comm_port ( int port )
{
	switch( port ) {
	    case COM1 :
		return &com1 ;
	    case COM2 :
		return &com2 ;
	}
	return NULL ;
}


COMM * comm_free ( void )
{
	if ( com1.irq_bit == 0 ) return &com1 ;
	if ( com2.irq_bit == 0 ) return &com2 ;
	return NULL ;
}


int comm_status ( COMM *sio )
{
	Byte	lsr	;

	lsr = ((inportb(sio->base+LSR)^TxReady)|sio->cLSR) ^ TxReady ;
	sio->cLSR = 0 ;
	return ((unsigned)lsr<<8)|sio->cMSR ;
}


void comm_close_all ( void )
{
	comm_close(&com1,1) ;
	comm_close(&com2,1) ;
}


void comm_clear ( COMM *sio )
{
	int	orgFC	;

	if ( sio->cFCR & 1 ) {	    /* 16550 enable FIFO */
		outportb(sio->base+FCR,sio->cFCR|0x06) ;
		nop() ;
		outportb(sio->base+FCR,sio->cFCR) ;
	}
	ENTER
	sio->inpbuf.filled = sio->inpbuf.rptr = sio->inpbuf.wptr = 0 ;
	sio->outbuf.filled = sio->outbuf.rptr = sio->outbuf.wptr = 0 ;
	EXIT
	orgFC = sio->flow ;
	sio->flow = ~orgFC ;
	change_protocol(sio,orgFC) ;
}


int comm_open ( COMM *sio, long bps, char *fmt )
{
	if ( sio == NULL ) return -1 ;

	if ( sio->bps == sio->mbps ) sio->mbps = 0 ;
	sio->bps = bps ;

	if ( fmt == NULL ) {	/* default: 8N1 */
		sio->cLCR = Data8 | ParityNone | Stop1 ;
	} else {
		int	lcr	;
		if ( !(fmt[0] >= '5' && fmt[0] <= '8') ) return NULL ;
		if ( !(fmt[2] >= '1' && fmt[2] <= '2') ) return NULL ;
		lcr = (fmt[0]-'5') + (fmt[2]-'1') * 4 ;
		switch( fmt[1] ) {
		    case 'N' :
			lcr |= ParityNone ;
			break ;
		    case 'E' :
			lcr |= ParityEven ;
			break ;
		    case 'O' :
			lcr |= ParityOdd ;
			break ;
		    case 'M' :
			lcr |= ParityMark ;
			break ;
		    case 'S' :
			lcr |= ParitySpace ;
			break ;
		    default :
			return NULL ;
		}
		sio->cLCR = lcr ;	/* Parity & Stop bits & Data bits */
	}

	if ( install_port(sio) < 0 ) {
		outportb(sio->base+TBR,0) ;
		if ( install_port(sio) < 0 ) return -1 ;
	}
	if ( sio->bps <= 2400 && sio->fifo_size != 0 )
		change_fifosize(sio,0,0) ;
	comm_clear(sio) ;
	return 0 ;
}


int comm_read ( COMM *sio, void *Buffer, int length )
{
	int	rlen	;
	Byte	*buffer = (Byte*)Buffer ;

	rlen = queue_length(&sio->inpbuf) ;
	if ( rlen == 0 ) return 0 ;
	if ( rlen > 1 ) {
		ENTER
		if ( length > rlen ) length = rlen ;
		for ( rlen = 1 ; rlen < length ; rlen++ ) {
			*buffer++ = queue_get(&sio->inpbuf) ;
		}
		*buffer = get_port(sio) ;
		EXIT
	} else {
		*buffer = get_port(sio) ;
	}

	return rlen ;
}


int comm_getc ( COMM *sio )
{
	return get_port(sio) ;
}


int comm_write ( COMM *sio, const void *Buffer, int length )
{
	int	nFree	;
	int	wlen = 0 ;
	Bptr	buffer = (Bptr)Buffer ;

	while ( length > 0 ) {
		nFree = sio->outbuf.size - sio->outbuf.filled ;
		if ( nFree == 0 ) {	/* full */
			if ( sio->taskComm )
				(sio->taskComm)(sio,TASK_WDELAY,125) ;
			else
				delay(125) ; /* sleep 0.125 second */
			nFree = sio->outbuf.size - sio->outbuf.filled ;
			if ( nFree == 0 ) break ;
		}

		ENTER
		if ( nFree > length ) nFree = length ;
		length -= nFree ;
		wlen += nFree ;
		do {
			queue_put(&sio->outbuf,*buffer++) ;
		} while ( --nFree > 0 ) ;
		if ( GetBit(sio->sts,S_xmt_pending) == 0 ) xmt_int(sio) ;
		EXIT
	}

	return wlen ;
}


int comm_puts ( COMM *sio, const void *Buffer )
{
	return comm_write(sio,Buffer,strlen((const char*)Buffer)) ;
}


int comm_query ( COMM *sio )
{
	return queue_length(&sio->inpbuf) ;
}


int comm_flow ( COMM *sio, int flow )
{
	return change_protocol(sio,flow) ;
}


void comm_RTS ( COMM *sio, int onoff )
{
	sio->cMCR = (sio->cMCR & ~RTS) | (onoff?RTS:0) ;
	outportb(sio->base+MCR,sio->cMCR) ;
}


void comm_DTR ( COMM *sio, int onoff )
{
	sio->cMCR = (sio->cMCR & ~DTR) | (onoff?DTR:0) ;
	outportb(sio->base+MCR,sio->cMCR) ;
}


void comm_disconnect ( COMM *sio, long millisec ) /* reset DTR singal */
{
	if ( sio->cMCR & DTR ) {
		comm_DTR(sio,0) ;
		if ( sio->taskComm )
			(sio->taskComm)(sio,TASK_DELAY,millisec) ;
		else
			delay(millisec) ;
		comm_DTR(sio,1) ;
	}
}


void comm_setbps ( COMM *sio, long bps )
{
	sio->bps = bps ;
	set_bps(sio) ;
}


void comm_break ( COMM *sio, long millisec )
{
	outportb(sio->base+LCR,sio->cLCR|BreakActive) ;
	if ( sio->taskComm )
		(sio->taskComm)(sio,TASK_DELAY,millisec) ;
	else
		delay(millisec) ;
	outportb(sio->base+LCR,sio->cLCR&=~BreakActive) ;
}


void comm_fifosize ( COMM *sio, int TxSize, int RxSize )
{
	change_fifosize(sio,TxSize,RxSize) ;
}

/*****************************************************************************/
