/* cmos.c */

#include "pchw.h"

void cmos_write ( int port, int data )
{
	outportb(CMOS_PORT,port) ;
	outportb(CMOS_DATA,data) ;
}


int cmos_read ( int port )
{
	outportb(CMOS_PORT,port) ;
	return inportb(CMOS_DATA) ;
}


static int cmos_bcd ( void )
{
	return cmos_read(CMOS_STATUSB) & 0x02 ;
}


static void cmos_write_bcd ( int port, int data )
{
	cmos_write(port,cmos_bcd()?(data/10)<<4|(data%10):data) ;
}


static int cmos_read_bcd ( int port )
{
	int in = cmos_read(port) ;
	if ( cmos_bcd() )
		return (in >> 4) * 10 + (in & 15) ;
	else
		return in ;
}


int cmos_why ( void )
{
	return cmos_read(CMOS_STATUSC) ;
}


void cmos_get_date ( int *year, int *month, int *day )
{
	ENTER
	while ( cmos_read(CMOS_STATUSA) & 0x80 ) ;
	*year = cmos_read_bcd(CMOS_HUNDREDYEAR)*100 + cmos_read_bcd(CMOS_YEAR) ;
	*month = cmos_read_bcd(CMOS_MONTH) ;
	*day = cmos_read_bcd(CMOS_DAY) ;
	EXIT
}


void cmos_get_time ( int *hour, int *minute, int *second )
{
	ENTER
	while ( cmos_read(CMOS_STATUSA) & 0x80 ) ;
	*hour = cmos_read_bcd(CMOS_HOUR) ;
	*minute = cmos_read_bcd(CMOS_MINUTE) ;
	*second = cmos_read_bcd(CMOS_SECOND) ;
	EXIT
}


void cmos_set_date ( int year, int month, int day )
{
	ENTER
	while ( cmos_read(CMOS_STATUSA) & 0x80 ) ;
	cmos_write_bcd(CMOS_HUNDREDYEAR,year/100) ;
	cmos_write_bcd(CMOS_YEAR,year%100) ;
	cmos_write_bcd(CMOS_MONTH,month) ;
	cmos_write_bcd(CMOS_DAY,day) ;
	EXIT
}


void cmos_set_time ( int hour, int minute, int second )
{
	ENTER
	while ( cmos_read(CMOS_STATUSA) & 0x80 ) ;
	cmos_write_bcd(CMOS_HOUR,hour) ;
	cmos_write_bcd(CMOS_MINUTE,minute) ;
	cmos_write_bcd(CMOS_SECOND,second) ;
	EXIT
}


void cmos_irq8 ( int irq8_flags )
{
static	signed char oldS8259 = -1 ;
	unsigned char b ;

	ENTER
	if ( irq8_flags ) {
		b = inportb(0x21) ;
		outportb(0x21,b&~0x04) ;	/* enable slaver 8259, IRQ2 */
		b = inportb(0xa1) ;
		if ( oldS8259 < 0 ) oldS8259 = b & 0x01 ;
		outportb(0xa1,b&~0x01) ;	/* IRQ8:Real Time Clock */
	} else {
		b = inportb(0xa1) | (oldS8259 & 0x01) ;
		outportb(0xa1,b) ;		/* restore IRQ8 	*/
	}

	cmos_why() ; // For some reason, this is necessary!

/* ...	b = cmos_read(CMOS_STATUSA) & 0xf0 ;	.. save divider */
	b = 0x20 ;				/* divider -- 32.768Khz  */
	cmos_write(CMOS_STATUSA,b|0x06) ;	/* 1024 ticks per second */

	b = cmos_read(CMOS_STATUSB) ;
	b &= ~(IRQ8_TIMER|IRQ8_ALARM|IRQ8_SECOND) ;
	b |= (IRQ8_TIMER|IRQ8_ALARM|IRQ8_SECOND) & irq8_flags ;
	cmos_write(CMOS_STATUSB,b) ;
	EXIT
}


void cmos_set_alarm ( int hour, int minute, int second )
{
	ENTER
	while ( cmos_read(CMOS_STATUSA) & 0x80 ) ;
	cmos_write_bcd(CMOS_SECOND+1,second) ;
	cmos_write_bcd(CMOS_MINUTE+1,minute) ;
	cmos_write_bcd(CMOS_HOUR  +1,hour) ;
	EXIT
}

