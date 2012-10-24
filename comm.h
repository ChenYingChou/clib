/* comm.h
*/

#if !defined(_COMM_H_)
#define _COMM_H_	1

#include "typedef.h"

#ifdef __cplusplus
extern "C" {
#else
	/* port */
	#define COM1			1
	#define COM2			2

	/* MCR: Modem Control Register */
	#define DTR			0x01
	#define RTS			0x02

	/* MSR: Modem Status Register */
	#define dCTS			0x01
	#define dDSR			0x02
	#define dRING			0x04
	#define dCD			0x08
	#define CTS			0x10
	#define DSR			0x20
	#define RING			0x40
	#define CD			0x80

	/* Flow Control */
	#define FC_XON_XOFF		0x01
	#define FC_CTS_RTS		0x02
	#define FC_NO_XMT_XON_XOFF	0x04		/* for HP only	*/

	/* Task Flag */
	#define TASK_RX 		0x01		/* Receive	*/
	#define TASK_TX 		0x02		/* Transmite	*/
	#define TASK_MS 		0x04		/* Modem Status */
	#define TASK_LS 		0x08		/* Line Status	*/

	#define TASK_INIT		1
	#define TASK_FINISH		2
	#define TASK_RDELAY		3		/* Read Delay	*/
	#define TASK_WDELAY		4		/* Write Delay	*/
	#define TASK_DELAY		5
#endif

#if !defined(_V_)
    #define _V_ volatile
#endif

typedef struct	{
	Byte	*ptr		;
	int	size		;
_V_	int	filled		;
_V_	int	wptr		;
_V_	int	rptr		;
}	QUEUE	;

typedef struct COMM COMM ;
typedef void (*TaskComm)(COMM*,int/*0:finish,1:init,2:write delay,3:delay*/,int) ;

struct	COMM {
	Word	base		; /* UART I/O base port 		*/
	Byte	irq		; /* IRQ number: 3,4,5,7,9,10,11,12,15	*/
	Byte	irq_bit 	; /* IRQ mask bit: 1 << (irq & 0x07)	*/
	long	bps		; /* 1200 - 115200			*/
	long	mbps		; /* Modem bps: carrier speed (DCE-DCE) */

	QUEUE	inpbuf		; /* Received Queue			*/
	QUEUE	outbuf		; /* Transmitted Queue			*/

	int	high_level	; /* High water level of Handshanking	*/
	int	low_level	; /* Low water level of Handshanking	*/

	int	fifo_size	; /* 1655x FIFO Tx size 		*/

	Byte	sts		; /* Flags: Run-time handshanking	*/
	Byte	flow		; /* Flow control			*/

	Byte	xIRQ		; /* Saved IRQ enable mask bit		*/
	Byte	xLCR		; /* Saved Line Control Register	*/
	Byte	xMCR		; /* Saved Modem Control Register	*/
	Byte	xIER		; /* Saved Interrupt Enable Register	*/
	Byte	xDLL		; /* Saved DLLSB			*/
	Byte	xDLM		; /* Saved DLLMB			*/

	Byte	cLCR		; /* Current Line Control Register	*/
	Byte	cMCR		; /* Current Modem Control Register	*/
	Byte	cMSR		; /* Current Modem Status Register	*/
	Byte	cLSR		; /* Keep last LSR: !TxReady		*/
	Byte	cFCR		; /* Current FIFO Control Register	*/

	Byte	force_xmt_ready ; /* Used by interrupt service routine	*/

	TaskComm taskComm	; /* Task/Comm init/finish/... routines */
	void	*_qCOMM 	; /* Type should be (TQueue *)		*/
	int	*taskFlag	; /* Task Flags 			*/
} ;

/* commdrv.c */
void	comm_close_all	( void ) ;
COMM *	comm_free	( void ) ;
COMM *	comm_port	( int port ) ;
int	comm_open	( COMM *sio, long bps, char *fmt ) ;
int	comm_close	( COMM *sio, int restore ) ;
int	comm_getc	( COMM *sio ) ;
int	comm_read	( COMM *sio, void *Buffer, int length ) ;
int	comm_putc	( COMM *sio, int ch ) ;
int	comm_write	( COMM *sio, const void *Buffer, int length ) ;
int	comm_puts	( COMM *sio, const void *Buffer ) ;
int	comm_status	( COMM *sio ) ;
int	comm_query	( COMM *sio ) ;
int	comm_flow	( COMM *sio, int flow ) ;
void	comm_clear	( COMM *sio ) ;
void	comm_RTS	( COMM *sio, int onoff ) ;
void	comm_DTR	( COMM *sio, int onoff ) ;
void	comm_setbps	( COMM *sio, long bps ) ;
void	comm_break	( COMM *sio, long millisec ) ;
void	comm_disconnect ( COMM *sio, long millisec ) ;
void	comm_fifosize	( COMM *sio, int TxSize, int RxSize ) ;

/* commirq.c */
int	comm_irq	( unsigned base ) ;

/* taskcomm.c */
void	task_comm_init	( void ) ;
int	comm_wait	( COMM *sio, unsigned long millisec ) ;
int	comm_wait_key	( COMM *sio, unsigned long millisec ) ;
void	qCOMM_add	( COMM *sio, int taskFlag ) ;

#ifdef __cplusplus
};

class	Comm	{
	COMM		*sio	; /* pointer to COMM			*/
	int		isOpen	;
	int		status	; /* LSR | MSR, set by Status() 	*/
	int		xBase	; /* original base			*/
	int		xIrq	; /* original irq			*/

    public:
	enum	PORT	{ COM1=1, COM2, COM3, COM4			} ;
	enum	FC	{ FC_XON_XOFF=1, FC_CTS_RTS=2, FC_NO_XMT_XON_XOFF=4 } ;
	enum	MCR	{ DTR=1, RTS=2, OUT1=4, OUT2=8, LOOP=16 	} ;
	enum	LSR	{ RxReady=1, DataOverRun=2, ParityError=4,
			  FrameError=8, BreakSignal=16, TxReady=32,
			  TxShiftReady=64, TimeOut=128			} ;
	enum	MSR	{ dCTS=1, dDSR=2, dRING=4, dCD=8,
			  CTS=16, DSR=32, RING=64, CD=128		} ;

	Comm ( int port ) ;
	Comm ( int base, int irq ) ;
	~Comm ( void ) ;

	void	Close		( int isRestore=0 )			;
	int	Open		( long BPS, char *fmt=0 )		;
	int	Open		( void )				;
	void	SetPort 	( int port )				;
	void	SetBaseIrq	( int base, int irq )			;
	int	Status		( void )				;

	operator COMM * 	(      ) { return sio			;}
	int	isOK		( void ) { return sio != 0		;}

	void	Clear		( void ) { comm_clear(sio)		;}
	void	ClearBuffer	( void ) { comm_clear(sio)		;}
	int	IncomeLen	( void ) { return sio->inpbuf.filled	;}
	int	OutcomeLen	( void ) { return sio->outbuf.filled	;}
	int	Getc		( void ) { return comm_getc(sio)	;}
	int	Putc		( char ch ) { return comm_putc(sio,ch)	;}

	int	GetBase 	( void ) { return sio->base		;}
	int	GetIRQ		( void ) { return sio->irq		;}
	int	GetFlowControl	( void ) { return sio->flow		;}

	int	isFifo		( void ) { return sio->cFCR & 1 	;}
	int	GetFifoTxSize	( void ) { return sio->fifo_size	;}
	int	GetFifoRxSize	( void )				;
	void	SetFifoTxSize	( int TxSize )				;
	void	SetFifoRxSize	( int RxSize )				;

	long	GetBps		( void ) { return sio->bps		;}
	void	SetBps		( long bps ) { comm_setbps(sio,bps)	;}

	long	GetCarrierBps	( void ) { return sio->mbps		;}
	void	SetCarrierBps	( long bps ) { sio->mbps = bps		;}

	void	SetRTS		( int OnOff ) { comm_RTS(sio,OnOff)	;}
	void	SetDTR		( int OnOff ) { comm_DTR(sio,OnOff)	;}
	int	isRTS		( void ) { return sio->cMCR&RTS  ?ON:OFF;}
	int	isDTR		( void ) { return sio->cMCR&DTR  ?ON:OFF;}

	int	isdCD		( void ) { return sio->cMSR&dCD  ?ON:OFF;}
	int	isdRING 	( void ) { return sio->cMSR&dRING?ON:OFF;}
	int	isdDSR		( void ) { return sio->cMSR&dDSR ?ON:OFF;}
	int	isdCTS		( void ) { return sio->cMSR&dCTS ?ON:OFF;}
	int	isCD		( void ) { return sio->cMSR&CD	 ?ON:OFF;}
	int	isRING		( void ) { return sio->cMSR&RING ?ON:OFF;}
	int	isDSR		( void ) { return sio->cMSR&DSR  ?ON:OFF;}
	int	isCTS		( void ) { return sio->cMSR&CTS  ?ON:OFF;}

	void	Disconnect	( long millisec=1000 )
	{
		comm_disconnect(sio,millisec) ;
	}

	void	Break		( long millisec=500 )
	{
		comm_break(sio,millisec) ;
	}

	void	SetFlowControl	( int flowControl )
	{
		comm_flow(sio,flowControl) ;
	}

	int	Read		( void *buffer, int length )
	{
		return comm_read(sio,buffer,length) ;
	}

	int	Puts		( const void *buffer )
	{
		return comm_puts(sio,buffer) ;
	}

	int	Write		( const void *buffer, int length )
	{
		return comm_write(sio,buffer,length) ;
	}

	int	isBreak 	( void )
	{
		return status & (BreakSignal<<8) ? ON : OFF ;
	}

	int	isFrameError	( void )
	{
		return status & (FrameError<<8) ? ON : OFF ;
	}

	int	isParityError	( void )
	{
		return status & (ParityError<<8) ? ON : OFF ;
	}

	int	isDataOverRun	( void )
	{
		return status & (DataOverRun<<8) ? ON : OFF ;
	}

	int	isRx ( void )
	{
		return status & (RxReady<<8) ? ON : OFF ;
	}

	int	isTx ( void )
	{
		return status & (TxReady<<8) ? OFF : ON ;
	}

}	;

#endif	/* __cplusplus */

#endif	/* _COMM_H_ */
