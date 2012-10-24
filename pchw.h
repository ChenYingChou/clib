/* pchw.h */

#if !defined(_PCHW_H_)
#define _PCHW_H_

#include <dos.h>

#if defined(__DJGPP__)
    /*
    #define ENABLE()			enable()
    #define DISABLE()			disable()
    */
    inline static void ENABLE ( void )
    {
	asm("sti") ;
    }

    inline static int DISABLE ( void )
    {
	int	isEnable ;
	asm("pushfl;popl %%eax;and $512,%%eax;jz 1f;cli;1:"
	    : "=a" (isEnable) ) ;
	return isEnable ;
    }
#elif defined(__WATCOMC__)
    #include <conio.h>

    #define inportb(p)			inp(p)
    #define outportb(p,v)		outp(p,v)

    void ENABLE ( void ) ;
    #pragma aux ENABLE = "sti" ;

    int DISABLE ( void ) ;
    #if defined(__386__)
	#pragma aux DISABLE =\
		    "pushfd"            \
		    "pop    eax"        \
		    "and    eax,0200h"  \
		    "jz     skip"       \
		    "cli"               \
		"skip:"                 \
		    value [eax] 	;
    #else
	#pragma aux DISABLE =\
		    "pushf"             \
		    "pop    ax"         \
		    "and    ax,0200h"   \
		    "jz     skip"       \
		    "cli"               \
		"skip:"                 \
		    value [ax]		;
    #endif
#else /* __BORLANDC__ */
    #define ENABLE()			enable()
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* CMOS: cmos.c ***************************************************************/
    #define CMOS_PORT		    0x70
    #define CMOS_DATA		    0x71

    /* Clock ports, check status regs for BCD mode or binary mode. */
    #define CMOS_SECOND 	    0x00
    #define CMOS_MINUTE 	    0x02
    #define CMOS_HOUR		    0x04
    #define CMOS_WEEKDAY	    0x06
    #define CMOS_DAY		    0x07
    #define CMOS_MONTH		    0x08
    #define CMOS_YEAR		    0x09
    #define CMOS_STATUSA	    0x0A
    #define CMOS_STATUSB	    0x0B
    #define CMOS_STATUSC	    0x0C
    #define CMOS_STATUSD	    0x0D
    #define CMOS_HUNDREDYEAR	    0x32

    /* BIOS config ports */
    #define CMOS_DIAGNOSE	    0x0E
    #define CMOS_FLOPPIES	    0x10 /* drive 2 in lower 4 bits, drive 1 in higher. */
    #define CMOS_HDD1		    0x11
    #define CMOS_HDD2		    0x12
    #define CMOS_CONFIG 	    0x14 /* as per BIOS function 0x11 */

    /* Floppy codes */
    #define FLOP_NONE		    0x00
    #define FLOP_360K		    0x01
    #define FLOP_1_2M		    0x02
    #define FLOP_720K		    0x03
    #define FLOP_1_44M		    0x04

    /* Diagnostic bits */
    #define ERROR_CLOCK 	    0x04
    #define ERROR_HDD		    0x08
    #define ERROR_MEMORY	    0x10
    #define ERROR_CONFIG	    0x20
    #define ERROR_CHECKSUM	    0x40
    #define ERROR_BATTERY	    0x80

    /* Config port bits */
    #define CONFIG_NODISKS	    0x01
    #define CONFIG_MATH_PROC	    0x02
    #define CONFIG_MODE_MASK	    0x18
	/* Those two bits are 1 for 40 column color, 2 for 80 column color, or
	   3 for 80 column mono. */

    void cmos_write (int port, int data ) ;
    int  cmos_read ( int port ) ;

    void cmos_get_date ( int *year, int *month, int *day ) ;
    void cmos_get_time ( int *hour, int *minute, int *second ) ;
    void cmos_set_date ( int year, int month, int day ) ;
    void cmos_set_time ( int hour, int minute, int second ) ;
    void cmos_set_alarm ( int hour, int minute, int second ) ;
    void cmos_irq8 ( int irq8_flags ) ;
    int  cmos_why ( void ) ; /* why irq8 occured, returns one (or more?) of  */
			     /* IE, assume cmos_why returns a bitwise OR of  */
			     /* one or more of these constants. 	     */
    #define IRQ8_SECOND 0x10
    #define IRQ8_ALARM	0x20
    #define IRQ8_TIMER	0x40


/* TIMER: timer.c *************************************************************/
    /* Clock in HZ */
    #define TIMER_CLOCK 0x1234DDL

    /* Channel 0 - IRQ 0 (INT 8)   */
    /* Channel 1 - Memory Refresh  */
    /* Channel 2 - Speaker Channel */

 /*****************************************************************************
  * MODE 0 - Interrupt on Terminal Count
  * 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
  * When this mode is set the output will be low. Loading the count register
  * with a value will cause the output to remain low and the counter will start
  * counting down. When the counter reaches 0 the output will go high and remain
  * high until the counter is reprogrammed. The counter will continue to count
  * down after terminal count is reached. Writing a value to the count register
  * during counting will stop the counter, writing a second byte starts the
  * new count.
  *
  * MODE 1 - Programmable One-Shot
  * 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
  * The output will go low once the counter has been loaded, and will go high
  * once terminal count has been reached. Once terminal count has been reached
  * it can be triggered again.
  *
  * MODE 2 - Rate Generator
  * 컴컴컴컴컴컴컴컴컴컴컴�
  * A standard divide-by-N counter. The output will be low for one period of the
  * input clock then it will remain high for the time in the counter. This cycle
  * will keep repeating.
  *
  * MODE 3 - Square Wave Rate Generator
  * 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴�
  * Similar to mode 2, except the ouput will remain high until one half of the
  * count has been completed and then low for the other half.
  *
  * MODE 4 - Software Triggered Strobe
  * 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
  * After the mode is set the output will be high. Once the count is loaded it
  * will start counting, and will go low once terminal count is reached.
  *
  * MODE 5 - Hardware Triggered Strobe
  * 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
  * Hardware triggered strobe. Similar to mode 5, but it waits for a hardware
  * trigger signal before starting to count.
  *
  * Modes 1 and 5 require the PIT gate pin to go high in order to start
  * counting. I'm not sure if this has been implemented in the PC.
  *****************************************************************************/

    /* Set up a channel. Default mode for channel 0 is 2, count 0 */
    void timer_setup ( int chan, int mode, unsigned int count ) ;

    void timer_install ( void (*handler)(void), unsigned int ticksPerSec );
    void timer_remove ( void ) ;


/* KEYBOARD: kbd.c ************************************************************/
    #define SCAN_ESC		1
    #define SCAN_1		2
    #define SCAN_2		3
    #define SCAN_3		4
    #define SCAN_4		5
    #define SCAN_5		6
    #define SCAN_6		7
    #define SCAN_7		8
    #define SCAN_8		9
    #define SCAN_9		10
    #define SCAN_0		11
    #define SCAN_MINUS		12
    #define SCAN_EQUALS 	13
    #define SCAN_BKSP		14
    #define SCAN_TAB		15
    #define SCAN_Q		16
    #define SCAN_W		17
    #define SCAN_E		18
    #define SCAN_R		19
    #define SCAN_T		20
    #define SCAN_Y		21
    #define SCAN_U		22
    #define SCAN_I		23
    #define SCAN_O		24
    #define SCAN_P		25
    #define SCAN_LSB		26	/* [ */
    #define SCAN_RSB		27	/* ] */
    #define SCAN_RETURN 	28
    #define SCAN_LCTRL		29
    #define SCAN_A		30
    #define SCAN_S		31
    #define SCAN_D		32
    #define SCAN_F		33
    #define SCAN_G		34
    #define SCAN_H		35
    #define SCAN_J		36
    #define SCAN_K		37
    #define SCAN_L		38
    #define SCAN_SEMI		39	/* ; */
    #define SCAN_QUOTER 	40	/* ' */
    #define SCAN_QUOTEL 	41	/* ` */
    #define SCAN_LSHFT		42
    #define SCAN_Z		44
    #define SCAN_X		45
    #define SCAN_C		46
    #define SCAN_V		47
    #define SCAN_B		48
    #define SCAN_N		49
    #define SCAN_M		50
    #define SCAN_COMMA		51	/* , */
    #define SCAN_DOT		52	/* . */
    #define SCAN_SLASH		53	/* / */
    #define SCAN_RSHFT		54
    #define SCAN_KPD_AST	55	/* KEYPAD * */
    #define SCAN_LALT		56
    #define SCAN_SPACE		57
    #define SCAN_CAPSLOCK	58
    #define SCAN_F1		59
    #define SCAN_F2		60
    #define SCAN_F3		61
    #define SCAN_F4		62
    #define SCAN_F5		63
    #define SCAN_F6		64
    #define SCAN_F7		65
    #define SCAN_F8		66
    #define SCAN_F9		67
    #define SCAN_F10		68
    #define SCAN_NUMLOCK	69
    #define SCAN_SCRLOCK	70
    #define SCAN_KPD_7		71
    #define SCAN_KPD_8		72
    #define SCAN_KPD_9		73
    #define SCAN_KPD_MINUS	74
    #define SCAN_KPD_4		75
    #define SCAN_KPD_5		76
    #define SCAN_KPD_6		77
    #define SCAN_KPD_PLUS	78
    #define SCAN_KPD_1		79
    #define SCAN_KPD_2		80
    #define SCAN_KPD_3		81
    #define SCAN_KPD_0		82
    #define SCAN_KPD_DOT	83
    #define SCAN_BKSLASH	86	/* \ */
    #define SCAN_F11		87
    #define SCAN_F12		88

    #define SCAN_KPD_ENTER	89
    #define SCAN_RCTRL		90
    #define SCAN_PRTSC		84
    #define SCAN_KPD_SLASH	92
    #define SCAN_RALT		93
    #define SCAN_HOME		94
    #define SCAN_UP		95
    #define SCAN_PGUP		96
    #define SCAN_LEFT		97
    #define SCAN_RIGHT		98
    #define SCAN_END		99
    #define SCAN_DOWN		100
    #define SCAN_PGDN		101
    #define SCAN_INSERT 	102
    #define SCAN_DELETE 	103
    #define SCAN_PAUSE		104

    /* look at this array for the scan codes of currently pushed keys. */
    extern volatile char _kbd_pressed[105] ;
    extern volatile char _kbd_pause ;

    void kbd_install ( int (*handler)(int scan,int state), int passon ) ;
    void kbd_remove  ( void ) ;       /* remove handler */
    int  kbd_pass    ( int passon ) ; /* 0:off, 1:on, -1:get old status */


/* irq: irq.c *****************************************************************/

#if defined(__DJGPP__)

    #define LOCKED_CODE 	    __attribute__((section(".ltxt")))
    #define LOCKED_DATA 	    __attribute__((section(".ldat"),nocommon))
    #define LOCK_VARIABLE(x)	    _lock_data((void*)&x,sizeof(x))
    #define LOCK_FUNCTION(x)	    _lock_code(x,(long)x##_end-(long)x)

    #define _lock_data(addr,size)   _go32_dpmi_lock_data(addr,size)
    #define _lock_code(addr,size)   _go32_dpmi_lock_code(addr,size)
    #define _pm_getvect(vect,addr)  __dpmi_get_protected_mode_interrupt_vector(vect,addr)
    #define _pm_setvect(vect,addr)  __dpmi_set_protected_mode_interrupt_vector(vect,addr)

    /* in dpmi.h */
    int _go32_dpmi_lock_code ( void *_lockaddr, unsigned long _locksize ) ;
    int _go32_dpmi_lock_data ( void *_lockaddr, unsigned long _locksize ) ;

#elif defined(__386__) /* WATCOM C++/386 */

    #define LOCKED_CODE 	    /* not used */
    #define LOCKED_DATA 	    /* not used */
    #define LOCK_VARIABLE(x)	    _lock_data((void*)&x,sizeof(x))
    #define LOCK_FUNCTION(x)	    _lock_code(x,(long)x##_end-(long)x)

    /* in irq.c */
    int _lock_code ( void *_lockaddr, unsigned long _locksize ) ;
    int _lock_data ( void *_lockaddr, unsigned long _locksize ) ;

#else /* not need for DOS real-mode */

    #define LOCKED_CODE 	    /* nothing */
    #define LOCKED_DATA 	    /* nothing */
    #define LOCK_VARIABLE(x)	    /* nothing */
    #define LOCK_FUNCTION(x)	    /* nothing */

#endif

    #define END_OF_FUNCTION(x)	    void x##_end() { }

    int  _install_irq ( int irq, int (*handler)(void) ) ;
    void _remove_irq ( int irq ) ;

/******************************************************************************/

#if defined(__BORLANDC__)
    #define ENTER		{ int isEnable ; __emit__(156,88,37,0,2,116,1,250) ;\
				  isEnable=_AX ;
#else
    #define ENTER		{ int isEnable = DISABLE() ;
#endif

#define EXIT			if ( isEnable ) ENABLE() ; }

#ifdef __cplusplus
}
#endif

#endif

