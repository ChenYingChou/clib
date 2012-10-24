/*
 * $Log: HRTIMER.H $
 * Revision 1.0  1996-02-06 23:29:13-0800  YCCHEN
 * Initial revision
 *
**/

/* High Resolution Timer */

#if !defined(_HRTIMER_H_)
#define _HRTIMER_H_

/* #define TimerResolution	   1193181.667	*/
/* #define TimerResolution_1000    1193 	*/

#ifdef __cplusplus
extern "C" {
#endif

/* Reprogram the timer chip to allow 1 microsecond resolution */
void initializetimer ( void ) ;

/* Restore the timer chip to its normal state */
void restoretimer ( void ) ;

/* Read the timer with 1 microsecond resolution */
unsigned long readtimer ( void ) ;

/* Calculate time elapsed (microseconds) between Start and Stop */
unsigned long elapsedtime ( unsigned long start, unsigned long stop ) ;

/* Replace delay function of TC/BC */
void delay ( unsigned milliseconds ) ;

/* Release Current Virtual Machine's Time Slice: Int 2FH Function 1680H */
int os_idle ( void );

#ifdef __cplusplus
};
#endif

#endif	/* _HRTIMER_H_ */

