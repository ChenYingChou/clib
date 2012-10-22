/*
 * $Log: DOSDRV.H $
 * Revision 1.0  1996-02-06 23:29:13-0800  YCCHEN
 * Initial revision
 *
**/

#ifdef __cplusplus
extern "C" {
#endif

int get_all_drive( char *drv ) ;
/******************************************************************************/
/*  return number of DOS drives 					      */
/*  drv[0-25]: drive ('A'+n): present if drv[n] <> 0                          */
/******************************************************************************/

int get_drive_info( int drv, char info[] ) ;
/******************************************************************************/
/*  return less than zero if ('A'+drv): not exist                             */
/*	drv : 0-25, drive number of DOS ( A: is 0, B: is 1, ... )	      */
/*  info[0] : char of drive type, info[1..n]:ASCIZ of description	      */
/*	      'H' : HARD-DISK                                                 */
/*	      'F' : FLOPPY                                                    */
/*	      'R' : RAM-DISK                                                  */
/*	      'J' : JOIN                                                      */
/*	      'S' : SUBST                                                     */
/*	      'C' : CD-ROM                                                    */
/*	      'N' : NETWORK                                                   */
/******************************************************************************/

#ifdef __cplusplus
};
#endif
