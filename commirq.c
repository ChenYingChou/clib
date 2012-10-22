/* commirq.c */

#include "pchw.h"

#ifdef __cplusplus
extern "C" {
#endif
int	comm_irq	( unsigned base ) ;
#ifdef __cplusplus
};
#endif

int comm_irq ( unsigned base )
{
	/* returns: -1 if no intlevel found, or intlevel 0-15 */
	char	ier, mcr, imrm, imrs, maskm, masks, irqm, irqs ;

	ENTER			/* disable all CPU interrupts */
	ier = inportb(base+1) ; /* read IER */
	outportb(base+1,0) ;	/* disable all UART ints */
	while ( !(inportb(base+5)&0x20) ) ; /* wait for the THR to be empty */
	mcr = inportb(base+4) ; /* read MCR */
	outportb(base+4,(mcr&15)|8) ; /* connect UART to irq line (OUT2 on) */
	imrm = inportb(0x21) ;	/* read contents of master ICU mask register */
	imrs = inportb(0xA1) ;	/* read contents of slave ICU mask register */
	outportb(0xA0,0x0A) ;	/* next read access to 0xA0 reads out IRR */
	outportb(0x20,0x0A) ;	/* next read access to 0x20 reads out IRR */
	outportb(base+1,2) ;	/* let's generate interrupts... */
	maskm = inportb(0x20) ; /* this clears all bits except for the one */
	masks = inportb(0xA0) ; /* that corresponds to the int */
	outportb(base+1,0) ;	/* drop the int line */
	maskm &= ~inportb(0x20);/* this clears all bits except for the one */
	masks &= ~inportb(0xA0);/* that corresponds to the int */
	outportb(base+1,2) ;	/* and raise it again just to be sure... */
	maskm &= inportb(0x20) ;/* this clears all bits except for the one */
	masks &= inportb(0xA0) ;/* that corresponds to the int */
	outportb(0xA1,~masks) ; /* now let us unmask this interrupt only */
	outportb(0x21,~maskm) ;
	outportb(0xA0,0x0C) ;	/* enter polled mode ; Mike Surikov reported */
	outportb(0x20,0x0C) ;	/* that order is important with 586/PCI systems */
	irqs = inportb(0xA0) ;	/* and accept the interrupt */
	irqm = inportb(0x20) ;
	inportb(base+2) ;	/* reset transmitter interrupt in UART */
	outportb(base+4,mcr) ;	/* restore old value of MCR */
	outportb(base+1,ier) ;	/* restore old value of IER */
	if ( masks ) outportb(0xA0,0x20) ;   /* send an EOI to slave */
	if ( maskm ) outportb(0x20,0x20) ;   /* send an EOI to master */
	outportb(0x21,imrm) ;	/* restore old mask register contents */
	outportb(0xA1,imrs) ;
	EXIT

	if ( irqs & 0x80 )	/* slave interrupt occured */
		return (irqs&0x07)+8 ;
	if ( irqm & 0x80 )	/* master interrupt occured */
		return irqm&0x07 ;
	return -1 ;
}

