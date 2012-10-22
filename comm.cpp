/*
 * $Log$
 *
*/

#include "comm.h"

Comm::Comm ( int port )
{
	isOpen = 0 ;
	xBase = 0 ;
	xIrq = 0 ;
	sio = comm_free() ;
	if ( isOK() ) {
		xBase = sio->base ;
		xIrq  = sio->irq ;
		SetPort(port) ;
	}
}


Comm::Comm ( int base, int irq )
{
	isOpen = 0 ;
	xBase = 0 ;
	xIrq = 0 ;
	sio = comm_free() ;
	if ( isOK() ) {
		xBase = sio->base ;
		xIrq  = sio->irq ;
		SetBaseIrq(base,irq) ;
	}
}


Comm::~Comm ( void )
{
	if ( isOK() ) {
		Close(1) ;
		sio->base = xBase ;
		sio->irq  = xIrq ;
	}
}


void Comm::Close ( int isRestore )
{
	if ( isOpen ) {
		comm_close(sio,isRestore) ;
		isOpen = 0 ;
	}
}


int Comm::Open ( long BPS, char *fmt )
{
	if ( isOpen ) Close(0) ;
	isOpen = (comm_open(sio,BPS,fmt) == 0) ;
	return isOpen ;
}


int Comm::Open ( void )
{
	if ( isOpen ) isOpen = (comm_open(sio,GetBps(),0) == 0) ;
	return isOpen ;
}


void Comm::SetPort ( int port )
{
static	Word	Base[] = { 0x3f8,0x2f8,0x3e8,0x2e8 } ;
static	Byte	Irq [] = {     4,    3,    4,	 3 } ;
	if ( isOK() ) {
		int	n = port - COM1 ;
		if ( n >= 0 && n < COM4 ) {
			sio->base = Base[n] ;
			sio->irq  = Irq [n] ;
		}
	}
}


void Comm::SetBaseIrq ( int base, int irq )
{
	if ( isOK() ) {
		sio->base = base ;
		sio->irq  = (irq == 0 ? comm_irq(base) : irq) ;
	}
}


int Comm::Status ( void )
{
	status = comm_status(sio) ;
	if ( IncomeLen() ) status |= RxReady << 8 ;
	if ( OutcomeLen() ) status &= ~(TxReady << 8) ;
	return status ;
}


int Comm::GetFifoRxSize ( void )
{
	switch( sio->cFCR & 0xc0 ) {
	    case 0x00 :
		return 1 ;
	    case 0x40 :
		return 4 ;
	    case 0x80 :
		return 8 ;
	}
	return 16 ;
}


void Comm::SetFifoTxSize ( int TxSize )
{
	if ( isFifo() ) sio->fifo_size = TxSize ;
}


void Comm::SetFifoRxSize ( int RxSize )
{
	comm_fifosize(sio,sio->fifo_size,RxSize) ;
}

