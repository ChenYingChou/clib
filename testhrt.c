/* testhrt.c */

#include <stdio.h>
#include <stdlib.h>
#include <dos.h>
#include <bios.h>
#include <time.h>
#include "hrtimer.h"

int main ( int argc, char *argv[] )
{
	unsigned	interval = ( argc > 1 ? atoi(argv[1]) : 5000 ) ;
	clock_t 	x, y	;
	unsigned long	a, b, c ;
	int		i	;

	printf("Delay %u milliseconds\n",interval);

	x = clock() ;
	a = readtimer() ;

	delay(interval);	/* in hrtimer.c */

	b = readtimer() ;
	y = clock() ;
	printf("BIOS CLOCK : %lu-%lu=%lu --> %.6f\n",
		y,x,y-x,(double)(y-x)/CLOCKS_PER_SEC) ;
	c = elapsedtime(a,b) ;
	printf("TimerClock : %lu-%lu=%lu --> %lu.%06lu seconds\n",
		b,a,b-a,c/1000000,c%1000000) ;

	a = readtimer() ;
	for ( i = 1 ; i < 16 ; i++ ) b = readtimer() ;
	printf("readtimer() = %lu clocks\n",(b-a)>>4) ;
	
	return 0;
}

