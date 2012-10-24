/* irq.c */

#include <stdlib.h>

#if defined(__386__)	/* DJGPP or WATCOM C++/386 */

    #if defined(__DJGPP__)

	#include <dpmi.h>
	#include <sys/segments.h>
	#include "pchw.h"

	/* markers of the locked sections, defined by ld (see djgpp.lnk) */
	extern char sltext __asm__("sltext") ;
	extern char eltext __asm__("eltext") ;
	extern char sldata __asm__("sldata") ;
	extern char eldata __asm__("eldata") ;

	/* macros returning sizes of locked areas in bytes */
	#define LOCKED_TEXT_SIZE ((long)&eltext - (long)&sltext)
	#define LOCKED_DATA_SIZE ((long)&eldata - (long)&sldata)
	#define LOCKED_SIZE (LOCKED_TEXT_SIZE + LOCKED_DATA_SIZE)

	typedef __dpmi_paddr ISR ;

    #else /* WATCOM C++/386 */

	#include "pchw.h"

	typedef struct {
		unsigned long  offset32 ;
		unsigned int   selector ; // -zp4 if unsigned short
	} __dpmi_paddr ;

	unsigned _my_cs ( void ) ;
	#pragma aux _my_cs =\
		"xor    eax,eax"\
		"mov    ax,cs"  \
		value [eax]	;

	unsigned _my_ds ( void ) ;
	#pragma aux _my_ds =\
		"xor    eax,eax"\
		"mov    ax,ds"  \
		value [eax]	;

	int dpmi_linear_region ( int func, unsigned long addr, unsigned long size ) ;
	#pragma aux dpmi_linear_region =\
		   "    mov     di,si                   "\
		   "    shr     esi,16                  "\
		   "    mov     cx,bx                   "\
		   "    shr     ebx,16                  "\
		   "    int     31h                     "\
		   "    setc    al                      "\
		   "    neg     al                      "\
		   "    movsx   eax,al                  "\
			parm	[eax] [ebx] [esi]	 \
			modify	[ebx ecx esi edi]	 \
			value	[eax] ;

	unsigned long dpmi_get_segment_base ( unsigned selector ) ;
	#pragma aux dpmi_get_segment_base=\
		   "    mov     ax,0006h                "\
		   "    int     31h                     "\
		   "    mov     eax,-1                  "\
		   "    jc      dpmi_ret                "\
		   "dpmi_ok:                            "\
		   "    mov     eax,ecx                 "\
		   "    shl     eax,16                  "\
		   "    mov     ax,dx                   "\
		   "dpmi_ret:                           "\
			parm	[ebx]			 \
			modify	[ecx edx]		 \
			value	[eax] ;


	int _pm_getvect ( int vect, __dpmi_paddr *_addr ) ;
	#pragma aux _pm_getvect =\
		   "    mov     ax,0204h                "\
		   "    int     31h                     "\
		   "    setc    al                      "\
		   "    neg     al                      "\
		   "    movsx   eax,al                  "\
		   "    mov     word ptr [esi+4],cx     "\
		   "    mov     dword ptr [esi],edx     "\
			parm	[ebx] [esi]		 \
			modify	[ecx edx]		 \
			value	[eax] ;

	int _pm_setvect ( int vect, __dpmi_paddr *_addr ) ;
	#pragma aux _pm_setvect =\
		   "    mov     cx,word ptr [esi+4]     "\
		   "    mov     edx,dword ptr [esi]     "\
		   "    mov     ax,0205h                "\
		   "    int     31h                     "\
		   "    setc    al                      "\
		   "    neg     al                      "\
		   "    movsx   eax,al                  "\
			parm	[ebx] [esi]		 \
			modify	[ecx edx]		 \
			value	[eax] ;

	static int _lock_memory ( unsigned selector, void *addr, unsigned long size )
	{
		unsigned long	laddr	;

		laddr = dpmi_get_segment_base(selector) ;
		if ( laddr == (unsigned long)(-1) ) return -1 ;
		return dpmi_linear_region(0x0600,laddr+(unsigned long)addr,size) ;
	}

	int _lock_data ( void *_lockaddr, unsigned long _locksize )
	{
		return _lock_memory(_my_ds(),_lockaddr,_locksize) ;
	}

	int _lock_code ( void *_lockaddr, unsigned long _locksize )
	{
		return _lock_memory(_my_cs(),_lockaddr,_locksize) ;
	}

 /****************************************************************************/
 /* DOS4G/W: In real-mode, interrupt doesn't autopass to protected-mode ISR, */
 /*	but _dos_setvect(autopass:08H-2eH,ISR) can get control in real-mode, */
 /*	so call _dos_setvect() to install ISR for WATCOM C/386. 	     */
 /****************************************************************************/
	typedef void (__interrupt __far *ISR)() ;

    #endif

#else	/* DOS real-mode */

    #include "pchw.h"

    #if defined(__WATCOMC__)
	typedef void (__interrupt __far *ISR)() ;
    #else	/* __BORLANDC__ */
	typedef void interrupt (far *ISR)() ;
    #endif

    #if defined(__SMALL__) || defined(__MEDIUM__) || defined(__TINY__)
	#define NormalizeEven(p)	    (unsigned char*)((unsigned)p&(~2))
    #elif defined(__WATCOMC__)	/* __COMPACT__ || __LARGE__ || __HUGE__ */
	unsigned char * NormalizeEven ( unsigned char * base ) ;
	#pragma aux NormalizeEven = \
			"and    ax,000eh"       \
			parm [dx ax]		\
			value [dx ax]		;
    #else	/* __BORLANDC__:  __COMPACT__ || __LARGE__ || __HUGE__ */
	static unsigned char * NormalizeEven ( unsigned char * base )
	{
		asm {
			mov	ax,word ptr base
			mov	dx,word ptr base+2
			push	ax
			shr	ax,1
			shr	ax,1
			shr	ax,1
			shr	ax,1
			add	dx,ax
			pop	ax
			and	ax,000eh
		}
		return (unsigned char*)MK_FP(_DX,_AX) ;
	}
    #endif

#endif

#if defined(__386__)
#pragma pack(4)
#endif
typedef struct {
	int	(*handler)(void) ;
	int	irq		 ;
	ISR	old_vector	 ;
}	IRQ_HANDLER	;
#if defined(__386__)
#pragma pack()
#endif

#define MAX_IRQS	4

#if defined(__386__)
	#define IRQ_STACKS	8	    /* must consistent with irqwrap.s */
	#define STACK_SIZE	4*1024
#elif defined(__WATCOMC__) || defined(__SMALL__) || defined(__MEDIUM__) || defined(__TINY__)
	#define IRQ_STACKS	MAX_IRQS    /* must consistent with irqwrap.s */
	#define STACK_SIZE	2*1024
    #if defined(__WATCOMC__)
	/* Always keep SS:(stacks) to SEG:(global data) */
	static unsigned char _isr_stack[IRQ_STACKS*STACK_SIZE] ;
    #endif
#else	/* __BORLANDC__ && Large Data module */
	#define IRQ_STACKS	8	    /* must consistent with irqwrap.s */
	#define STACK_SIZE	2*1024
#endif


IRQ_HANDLER	_irq_handler[MAX_IRQS] ;
unsigned char * _irq_stack[IRQ_STACKS] ;
static int	irq_virgin = 1 ;

extern void _irq_wrapper_0(void), _irq_wrapper_1(void),
	    _irq_wrapper_2(void), _irq_wrapper_3(void) ;

#if defined(__386__)
extern void _irq_wrapper_0_end(void) ;
#endif


/* _install_irq:
 *  Installs a hardware interrupt handler for the specified irq, allocating
 *  an asm wrapper function which will save registers and handle the stack
 *  switching. The C function should return zero to exit the interrupt with
 *  an iret instruction, and non-zero to chain to the old handler.
 */
int _install_irq ( int irq, int (*handler)(void) )
{
	int	c ;

	if ( irq_virgin ) {		/* first time we've been called? */
		unsigned char *p ;

#if defined(__WATCOMC__) && !defined(__386__)
		/* must keep SEGMENT(_isr_stack) == SS for C/L/H module */
		p = NormalizeEven(_isr_stack) + IRQ_STACKS*STACK_SIZE ;
#else
		p = (unsigned char*)malloc(IRQ_STACKS*STACK_SIZE) ;
		if ( p == NULL ) return -1 ;
    #if defined(__386__)
		_lock_data(p,IRQ_STACKS*STACK_SIZE) ;
		p = (unsigned char*)((int)(p+IRQ_STACKS*STACK_SIZE-15) & (~3)) ;
    #else
		p = NormalizeEven(p) + IRQ_STACKS*STACK_SIZE ;
    #endif
#endif
		for ( c = IRQ_STACKS ; --c >= 0 ; ) {
			_irq_stack[c] = p ;
			/* WATCOM C/Large data: p can't be normalized */
			p -= STACK_SIZE ;
		}

#if defined(__DJGPP__)
		_lock_data(&sldata,LOCKED_DATA_SIZE) ;
		_lock_code(&sltext,LOCKED_TEXT_SIZE) ;
#endif

		LOCK_VARIABLE(_irq_handler) ;
		LOCK_VARIABLE(_irq_stack) ;
		LOCK_FUNCTION(_irq_wrapper_0) ;

		for ( c = 0 ; c < MAX_IRQS ; c++ ) {
			_irq_handler[c].handler = NULL ;
			_irq_handler[c].irq = -1 ;
		}

		irq_virgin = 0 ;
	}

	for ( c = 0 ; c < MAX_IRQS ; c++ ) {
		if ( _irq_handler[c].handler == NULL ) {
			ISR	addr ;
			int	vector = irq + (irq < 8 ? 8 : 0x70-8) ;

			_irq_handler[c].handler = handler ;
			_irq_handler[c].irq = irq ;

#if defined(__DJGPP__)	/* or PMODE/W, but not DOS4G/W */
			addr.selector = _my_cs() ;
			switch ( c ) {
			    case 0 :
				addr.offset32 = (long)_irq_wrapper_0 ;
				break ;
			    case 1 :
				addr.offset32 = (long)_irq_wrapper_1 ;
				break ;
			    case 2 :
				addr.offset32 = (long)_irq_wrapper_2 ;
				break ;
			    case 3 :
				addr.offset32 = (long)_irq_wrapper_3 ;
				break ;
			}
			_pm_getvect(vector,&_irq_handler[c].old_vector) ;
			_pm_setvect(vector,&addr) ;
#else
			switch ( c ) {
			    case 0 :
				addr = (ISR)_irq_wrapper_0 ;
				break ;
			    case 1 :
				addr = (ISR)_irq_wrapper_1 ;
				break ;
			    case 2 :
				addr = (ISR)_irq_wrapper_2 ;
				break ;
			    case 3 :
				addr = (ISR)_irq_wrapper_3 ;
				break ;
			}
			_irq_handler[c].old_vector = _dos_getvect(vector) ;
			_dos_setvect(vector,addr) ;
#endif
			return 0 ;
		}
	}

	return -1 ;
}


/* _remove_irq:
 *  Removes a hardware interrupt handler, restoring the old vector.
 */
void _remove_irq ( int irq )
{
	int c, vector ;

	for ( c = 0 ; c < MAX_IRQS ; c++ ) {
		if ( _irq_handler[c].irq == irq ) {
			vector = irq + (irq < 8 ? 8 : 0x70-8) ;
#if defined(__DJGPP__)	/* or PMODE/W, but not DOS4G/W */
			_pm_setvect(vector,&_irq_handler[c].old_vector) ;
#else
			_dos_setvect(vector,_irq_handler[c].old_vector) ;
#endif
			_irq_handler[c].irq = -1 ;
			_irq_handler[c].handler = NULL ;
			break ;
		}
	}
}

