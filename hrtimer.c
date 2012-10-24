/*
 * $Log: HRTIMER.C $
 * Revision 1.1  1996-02-29 02:31:47-0800  ycchen
 * Support 386 mode & change some registers used
 *
 * Revision 1.0  1996-02-06 23:29:12-0800  YCCHEN
 * Initial revision
 *
**/

/* High Resolution Timer */

#include	<stdlib.h>

#if defined(__DJGPP__)
#include	<pc.h>
#include	<go32.h>
#define 	outp(port,data) 	outportb(port,data)
#define 	inp(port)		inportb(port)
#else
#include	<conio.h>
#endif

#if defined(__TURBOC__)
#include	<dos.h>
#endif

#include	"hrtimer.h"

#define ACTIVE_ON		0x01
#define ACTIVE_REGISTER 	0x80

#if defined(__WATCOMC__)

/* ..
	void NOP ( void ) ;
	#pragma aux NOP = "nop" ;
..... */
	#define NOP()		/* nothing */

    #if defined(__386__)

	int os_idle ( );
	#pragma aux os_idle = \
		"mov     ax,1680h     " \
		"int     2fh          " \
		"and     eax,00ffh    " \
		value [eax]		;

	unsigned long _readtimer_ ( void ) ;
	#pragma aux _readtimer_ = \
		"pushf                " \
		"cli                  " \
		"mov  al,00Ah         " \
		"out  020h,al         " \
		"mov  al,00h          " \
		"out  043h,al         " \
		"in   al,020h         " \
		"mov  ch,al           " \
		"in   al,040h         " \
		"mov  bl,al           " \
		"in   al,040h         " \
		"mov  bh,al           " \
		"not  bx              " \
		"in   al,021h         " \
		"mov  cl,al           " \
		"mov  al,0FFh         " \
		"out  021h,al         " \
		"mov  al,cl           " \
		"out  021h,al         " \
		"mov  ax,ds:[046Ch]   " \
		"popf                 " \
		"test ch,001h         " \
		"jz   done            " \
		"or   bh,bh           " \
		"jnz  done            " \
		"inc  ax              " \
	    "done:                    " \
		"shl  eax,16          " \
		"mov  ax,bx           " \
		value [eax]		\
		modify [ebx ecx]	;

	unsigned long _elapsedtime_ ( unsigned long, unsigned long ) ;
	#pragma aux _elapsedtime_ = \
		"sub     edx,eax      " \
		"xor     eax,eax      " \
		"shrd    eax,edx,2    " \
		"shr     edx,2        " \
		"mov     ebx,1281169059" \
		"div     ebx          " \
		"shl     edx,1        " \
		"adc     eax,0        " \
		parm [eax] [edx]	\
		value [eax]		\
		modify [ebx edx]	;

	unsigned long _toTimerResolution_ ( unsigned ) ;
	#pragma aux _toTimerResolution_ = \
		"mov     eax,78196353 " \
		"mul     edx          " \
		"shrd    eax,edx,16   " \
		parm [edx]		\
		value [eax]		\
		modify [edx]		;

    #else	// !defined(__386__)

	int os_idle ( );
	#pragma aux os_idle = \
		"mov     ax,1680h     " \
		"int     2fh          " \
		"xor     ah,ah        " \
		value [ax]		\
		modify [ax]		;

	unsigned long _readtimer_ ( void ) ;
	#pragma aux _readtimer_ = \
		"pushf                " \
		"cli                  " \
		"mov  dx,020h         " \
		"mov  al,00Ah         " \
		"out  dx,al           " \
		"mov  al,00h          " \
		"out  043h,al         " \
		"in   al,dx           " \
		"mov  ch,al           " \
		"in   al,040h         " \
		"mov  bl,al           " \
		"in   al,040h         " \
		"mov  bh,al           " \
		"not  bx              " \
		"in   al,021h         " \
		"mov  cl,al           " \
		"mov  al,00FFh        " \
		"out  021h,al         " \
		"mov  ax,040h         " \
		"mov  es,ax           " \
		"mov  dx,es:[06Ch]    " \
		"mov  al,cl           " \
		"out  021h,al         " \
		"popf                 " \
		"test ch,001h         " \
		"jz   done            " \
		"or   bh,bh           " \
		"jnz  done            " \
		"inc  dx              " \
	    "done:                    " \
		"mov ax,bx            " \
		value [dx ax]		\
		modify [bx cx es]	;

	unsigned long _elapsedtime_ ( unsigned long, unsigned long ) ;
	#pragma aux _elapsedtime_ = \
		"sub     bx,ax        " \
		"push    bx           " \
		"sbb     cx,dx        " \
		"mov     ax,cx        " \
		"push    ax           " \
		"mov     si,54925     " \
		"mul     si           " \
		"mov     bx,ax        " \
		"mov     cx,dx        " \
		"pop     ax           " \
		"mov     dx,27295     " \
		"mul     dx           " \
		"add     bx,dx        " \
		"adc     cx,0         " \
		"pop     ax           " \
		"mul     si           " \
		"mov     ax,dx        " \
		"mov     dx,cx        " \
		"add     ax,bx        " \
		"adc     dx,0         " \
		parm [dx ax] [cx bx]	\
		value [dx ax]		\
		modify [bx cx si]	;

	unsigned long _toTimerResolution_ ( unsigned ) ;
	#pragma aux _toTimerResolution_ = \
		"mov     ax,11906     " \
		"mul     bx           " \
		"mov     cx,dx        " \
		"mov     ax,1193      " \
		"mul     bx           " \
		"add     ax,cx        " \
		"adc     dx,0         " \
		parm [bx]		\
		value [dx ax]		\
		modify [cx]		;

    #endif	// if defined(__386__)

#elif defined(__DJGPP__)

	#define NOP()		/* nothing */

	#include <dpmi.h>
	int os_idle ( ) {
		__dpmi_regs r;
		r.x.ax = 0x1680;
		__dpmi_int(0x2f,&r);
		return r.h.al;
	}

#else	/* Borland C */

/* ..	#define NOP()		asm nop 	*/
	#define NOP()		/* nothing */

	int os_idle ( void ) {
		asm {
			mov	ax,1680h
			int	2fh
			xor	ah,ah
		}
		return _AX;
	}

#endif

static	unsigned char	active = 0	;

void restoretimer ( void )
{
	if ( (active & ACTIVE_REGISTER) == 0 ) return ;
	active = 0 ;

	outp(0x043,0x036) ;
	NOP() ;
	outp(0x040,0x000) ;
	NOP() ;
	outp(0x040,0x000) ;
}


void initializetimer ( void )
{
	active |= ACTIVE_ON ;
	if ( (active & ACTIVE_REGISTER) == 0 ) {
		active |= ACTIVE_REGISTER ;
		atexit(restoretimer) ;
	}

	outp(0x043,0x034) ;
	NOP() ;
	outp(0x040,0x000) ;
	NOP() ;
	outp(0x040,0x000) ;
}


unsigned long readtimer ( void )
{
	if ( (active & ACTIVE_ON) == 0 ) initializetimer() ;

#if defined(__WATCOMC__)
	return _readtimer_() ;
#elif defined(__DJGPP__)
	{
	register unsigned long result ;
	asm("   pushf
		cli
		movb	$0x0A,%%al
		outb	%%al,$0x20
		movb	$0x00,%%al
		outb	%%al,$0x43
		inb	$0x20,%%al
		movb	%%al,%%ch
		inb	$0x40,%%al
		movb	%%al,%%bl
		inb	$0x40,%%al
		movb	%%al,%%bh
		notw	%%bx
		inb	$0x21,%%al
		movb	%%al,%%cl
		movb	$0xff,%%al
		outb	%%al,$0x21
		movb	%%cl,%%al
		outb	%%al,$0x21
		pushw	%%es
		movw	%1,%%es
		movl	$0x046c,%%eax
		movw	%%es:(%%eax),%%ax
		popw	%%es
		popf
		testb	$0x01,%%ch
		jz	0f
		orb	%%bh,%%bh
		jnz	0f
		incw	%%ax
	    0:
		shll	$16,%%eax
		movw	%%bx,%%ax	"
	: "=a" (result)
	: "m" (_dos_ds)
	: "eax", "ebx", "ecx" ) ;
	return result ;
	}
#else	/* Borland C */
	asm	pushf		/* Save flags		  */
	asm	cli		/* Disable interrupts	  */
	asm	mov  dx,020h	/* Address PIC ocw3	  */
	asm	mov  al,00Ah	/* Ask to read irr	  */
	asm	out  dx,al
	asm	mov  al,00h	/* Latch timer 0	  */
	asm	out  043h,al
	asm	in   al,dx	/* Read irr		  */
	asm	mov  di,ax	/* Save it in DI	  */
	asm	in   al,040h	/* Counter --> bx	  */
	asm	mov  bl,al	/* LSB in BL		  */
	asm	in   al,040h
	asm	mov  bh,al	/* MSB in BH		  */
	asm	not  bx 	/* Need ascending counter */
	asm	in   al,021h	/* Read PIC imr 	  */
	asm	mov  si,ax	/* Save it in SI	  */
	asm	mov  al,00FFh	/* Mask all interrupts	  */
	asm	out  021h,al
	asm	mov  ax,040h	/* read low word of time  */
	asm	mov  es,ax	/* from BIOS data area	  */
	asm	mov  dx,es:[06Ch]
	asm	mov  ax,si	/* Restore imr from SI	  */
	asm	out  021h,al
	asm	popf		/* Restore flags	  */
	asm	mov  ax,di	/* Retrieve old irr	  */
	asm	test al,001h	/* Counter hit 0?	  */
	asm	jz   done	/* Jump if not		  */
	asm	cmp  bx,0FFh	/* Counter > 0x0FF?	  */
	asm	ja   done	/* Done if so		  */
	asm	inc  dx 	/* Else count int req.	  */
	    done:;
	asm	mov ax,bx	/* set function result	  */
#endif
}


unsigned long elapsedtime ( unsigned long start, unsigned long stop )
{
/************************************************************************/
/* x / 1.193181667 = ( H * 65536 + L ) / 1.193181667			*/
/*		   = H * 65536 / 1.193181667	   + L / 1.193181667	*/
/*		   = H * 54925 + H * 27295 / 65536 + L * 54925 / 65536	*/
/************************************************************************/
#if defined(__WATCOMC__)
	return _elapsedtime_(start,stop) ;
#elif defined(__DJGPP__)
	register unsigned long result ;
	asm("   subl    %%eax,%%edx
		xorl	%%eax,%%eax
		shrd	$2,%%edx,%%eax
		shrl	$2,%%edx
		movl	$1281169059,%%ebx
		divl	%%ebx
		shll	$1,%%edx
		adcl	$0,%%eax	"
	: "=a" (result)
	: "a" (start), "d" (stop)
	: "eax", "ebx", "edx" ) ;
	return result ;
#else	/* Borland C */
asm	mov	ax,word ptr stop
asm	sub	ax,word ptr start	; /* low */
asm	push	ax			; /* low word of (stop-start)	*/
asm	mov	ax,word ptr stop+2
asm	sbb	ax,word ptr start+2	; /* high */
asm	push	ax			; /* high word of (stop-start)	*/

asm	mov	si,54925		; /* 54925 = 65536/1.193181667	*/
asm	mul	si
asm	mov	bx,ax
asm	mov	cx,dx

asm	pop	ax
asm	mov	dx,27295		; /* (65536/1.193181667-54925)*65536 */
asm	mul	dx
asm	add	bx,dx
asm	adc	cx,0

asm	pop	ax			; /* low word of (stop-start)	*/
asm	mul	si			; /* 54925 = 65536/1.193181667	*/
asm	mov	ax,dx			; /* divide by 65536		*/

asm	mov	dx,cx
asm	add	ax,bx
asm	adc	dx,0
#endif
}


void delay ( unsigned milliseconds )
{
	unsigned long	start = readtimer()	;
	unsigned long	stop, curr, elapsed;

/* ...	stop = (unsigned long)milliseconds * TimerResolution_1000 +	*/
/* ...	       ( ((unsigned long)milliseconds * 11906) >> 16 ) ;	*/
/********************************************** ||||| *******************/
/*		  0.181667 * 65536 .=. 11906 <--+---+			*/
/*									*/
/*	x * 1193.181667 = x * ( 1193 + 0.181667 )			*/
/*			= x * 1193 + x * (65536*0.181667) / 65536	*/
/*			= x * 1193 + x * 11906 / 65536			*/
/************************************************************************/
#if defined(__WATCOMC__)
	stop = _toTimerResolution_(milliseconds) ;
#elif defined(__DJGPP__)
	asm("   movl    $78196353,%%eax
		mull	%%edx
		shrd	$16,%%edx,%%eax "
	: "=a" (stop)
	: "d" (milliseconds)
	: "edx" ) ;
#else	/* Borland C */
asm	mov	bx,milliseconds
asm	mov	ax,11906
asm	mul	bx
asm	mov	cx,dx			; /* (milliseconds*11906)>>16 */
asm	mov	ax,1193
asm	mul	bx
asm	add	ax,cx
asm	adc	dx,0
asm	mov	word ptr stop,ax
asm	mov	word ptr stop+2,dx
#endif

	//while ( readtimer() - start < stop ) ;

	curr = readtimer();
	stop -= (3*(curr-start)) / 2;
	while ((elapsed=curr-start) < stop) {
		if (stop - elapsed > 50000L) os_idle();
		curr = readtimer();
		if (curr < start) {
			curr = readtimer();
			if (curr < start) curr = start;
		}
	}
}

