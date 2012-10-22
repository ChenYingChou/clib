/* timer.c */

#include "pchw.h"

static unsigned long current_count LOCKED_DATA = 0x10000L ;

void timer_setup ( int chan, int mode, unsigned int count )
{
	if ( chan == 0 ) current_count = (count?count:0x10000L) ;
	outportb(0x43,chan<<6 | mode<<1 | 0x30) ;
	outportb(0x40+chan,count&0xff) ;
	outportb(0x40+chan,count>>8) ;
}

static void (*handler)(void) LOCKED_DATA ;
static volatile unsigned long tick_counter LOCKED_DATA = 0 ;

static int New08 ( void ) LOCKED_CODE ;
static int New08 ( void )
{
	tick_counter += current_count ;
	if ( handler ) handler() ;
	if ( tick_counter >= 0x10000 ) {
		tick_counter -= 0x10000 ;
		return 1 ;
	}
	return 0 ;
}


void timer_install ( void (*handl)(void), unsigned int ticksPerSecond )
{
	handler = handl ;
	_install_irq(0,New08) ;
	timer_setup(0,2,ticksPerSecond<18?0:TIMER_CLOCK/ticksPerSecond) ;
}


void timer_remove ( void )
{
	timer_setup(0,2,0) ;
	_remove_irq(0) ;
}

