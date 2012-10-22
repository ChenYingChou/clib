/*
 * $Log: DOSDRV.C $
 * Revision 1.0  1996-02-06 23:29:12-0800  YCCHEN
 * Initial revision
 *
**/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dos.h>
#include "dosdrv.h"

#if !defined(_Byte_)
#define _Byte_
	typedef unsigned int	Bit	;
	typedef unsigned char	Byte	;
	typedef unsigned short	Word	;
	typedef unsigned long	Dword	;
	typedef Byte *		Bptr	;
	typedef Word *		Wptr	;
    #if defined(__386__)
	#define FAR
	#define NEAR
    #elif defined(__WATCOMC__)
	#define FAR		__far
	#define NEAR		__near
    #else
	#define FAR		far
	#define NEAR		near
    #endif
	typedef Byte FAR *	Bfptr	;
	typedef Word FAR *	Wfptr	;
#endif

/******************************************************************************/
/* dosdrv.h -- From Undocumented DOS, 2nd edition (Addison-Wesley, 1993)      */
/******************************************************************************/
#pragma pack(1)

typedef struct dpb {		    /* Drive Parameter Block		*/
    Byte drive, unit;
    Word bytes_per_sect;
    Byte sectors_per_cluster;	    /* plus 1				*/
    Byte shift; 		    /* for sectors per cluster		*/
    Word boot_sectors;
    Byte copies_fat;
    Word max_root_dir, first_data_sector, highest_cluster;
    union {
	struct {
	    Byte sectors_per_fat;
	    Word first_dir_sector;  /* root dir 			*/
	    void FAR *device_driver;
	    Byte media_descriptor, access_flag;
	    struct dpb FAR *next;
	    Dword reserved;
	    } dos3;
	struct {
	    Word sectors_per_fat;	/* Word, not Byte!		*/
	    Word first_dir_sector;
	    void FAR *device_driver;
	    Byte media_descriptor, access_flag;
	    struct dpb FAR *next;
	    Dword reserved;
	    } dos4;
	} vers;
    } DPB;

typedef struct {
    char current_path[67];  /* current path				00h */
    Word flags; 	    /* NETWORK, PHYSICAL, JOIN, SUBST, CDROM	43h */
    DPB  FAR *dpb;	    /* pointer to Drive Parameter Block 	45h */
    union {
	struct {
	    Word start_cluster; /* root: 0000; never accessed: FFFFh	49h */
	    Dword unknown;
	    } LOCAL;	    /* if (! (cds[drive].flags & NETWORK))	*/
	struct {
	    Dword redirifs_record_ptr;
	    Word parameter;
	    } NET;	    /* if (cds[drive].flags & NETWORK)		*/
	} u;
    Word backslash_offset;  /* offset in current_path of '\'            4Fh */
    /* DOS4 fields for IFS	*/
    /* 7 extra bytes... 	*/
    } CDS;

/* flags (CDS offset 43h)	*/
#define NETWORK 	(1 << 15)
#define PHYSICAL	(1 << 14)
#define JOIN		(1 << 13)
#define SUBST		(1 << 12)
#define REDIR_NOT_NET	(1 << 7)    /* sometimes called CDROM		*/

#pragma pack()
/******************************************************************************/

#if defined(__TURBOC__)
#define DOS_INT 	geninterrupt(0x21)
#endif

/******************************************************************************/
static	Bfptr	cds		;
static	int	cdsSize 	;
static	int	lastDrv 	;
static	Byte	drvType[26]	;
/******************************************************************************/
#if defined(__WATCOMC__)

extern Word DOS_version ( void ) ;
#pragma aux DOS_version = \
		"mov    ax,3306h"       \
		"xor    bx,bx"          \
		"int    21h"            \
		value [bx]		\
		modify [ax]		;

#if	defined(__386__)

typedef struct	{
	Word	rdi, hdi	,
		rsi, hsi	,
		rbp, hbp	,
		r00, h00	,
		rbx, hbx	,
		rdx, hdx	,
		rcx, hcx	,
		rax, hax	;
	Word	flags		;
	Word	res		,
		rds		,
		rfs		,
		rgs		,
		rip		,
		rcs		,
		rsp		,
		rss		;
}	RealReg ;


extern Word DPMI_allocate_dos_memory ( Word size ) ;
#pragma aux DPMI_allocate_dos_memory =	\
		"mov    ax,0100h"       \
		"int    31h"            \
		"jnc    allocDOSret"    \
		"xor    ax,ax"          \
	    "allocDosret:"              \
		parm [bx]		\
		value [ax]		;


static Bfptr DOS_sysvars ( void )
{
	union REGPACK	r	;
	RealReg 	x	;

	memset(&x,0,sizeof(x)) ;
	x.rax = 0x5200 ;

	memset(&r,0,sizeof(r)) ;
	r.w.ax = 0x300 ;	/* DPMI simulate real-mode INT */
	r.w.bx = 0x0021 ;	/* DOS call */
	r.w.cx = 0 ;		/* # byte from stack */
	r.x.edi = FP_OFF(&x) ;
	r.x.es	= FP_SEG(&x) ;
	intr(0x31,&r) ; 	/* call DPMI */
	return (Bfptr)((x.res<<4)+x.rbx) ;	/* 1M: MK_FP(0,seg<<4+offset) */
}


static long DOS_assignList ( Word index, Bfptr local, Bfptr net )
{
static	Word		xDosSeg ;
	union REGS	r	;
	struct SREGS	s	;

	if ( xDosSeg == 0 ) xDosSeg = DPMI_allocate_dos_memory(256/16) ;

	if ( xDosSeg ) {
		union REGPACK	r	;
		RealReg 	x	;

		memset(&x,0,sizeof(x)) ;
		x.rax = 0x5f02 ;
		x.rbx = index ;
		x.rsi = 0 ;		/* offset of local */
		x.rdi = 128 ;		/* offset of net */
		x.rds = x.res = xDosSeg ;

		memset(&r,0,sizeof(r)) ;
		r.w.ax = 0x0300 ;
		r.w.bx = 0x0021 ;
		r.w.cx = 0 ;		/* # byte from stack */
		r.x.edi = FP_OFF(&x) ;
		r.x.es	= FP_SEG(&x) ;
		intr(0x31,&r) ; 	/* call DPMI */

		strcpy((char*)local,(char*)(xDosSeg*16)) ;
		strcpy((char*)net,(char*)(xDosSeg*16+128)) ;

		return x.flags & INTR_CF ? -1 : (x.rcx << 16) + x.rbx ;
	}

	/* PMODEW not support DOS function 0x5f */

	memset(&r,0,sizeof(r)) ;
	r.w.ax = 0x5f02 ;
	r.w.bx = index ;
	r.x.esi = FP_OFF(local) ;
	s.ds	= FP_SEG(local) ;
	r.x.edi = FP_OFF(net) ;
	s.es	= FP_SEG(net) ;
	int386x(0x21,&r,&r,&s) ;

	return r.w.cflag ? -1 : (r.w.cx << 16) + r.w.bx ;
}

#else

extern Bfptr DOS_sysvars ( void ) ;
#pragma aux DOS_sysvars = \
		"mov    ah,52h"         \
		"int    21h"            \
		value [es bx]		\
		modify [ax]		;

extern long DOS_assignList ( Word index, Bfptr local, Bfptr net ) ;
#pragma aux DOS_assignList = \
		"push   ds"             \
		"mov    ds,dx"          \
		"mov    ax,5f02h"       \
		"push   bp"             \
		"int    21h"            \
		"pop    bp"             \
		"pop    ds"             \
		"jnc    assignRet"      \
		"mov    cx,-1"          \
		"mov    bx,ax"          \
	    "assignRet:"                \
		parm [bx] [dx si] [es di]\
		value [cx bx]		\
		modify [ax dx]		;

#endif	/* __386__ */

#endif	/* __WATCOMC__ */
/******************************************************************************/

static void set_dos_env ( void )
{
	Bfptr	sysvars 		;
	int	drv_ofs, cds_ofs	;

#if defined(__TURBOC__)
	_AX = 0x3306 ;
	_BX = 0 ;
	DOS_INT ;
	drv_ofs = _BX ;
#else
	drv_ofs = DOS_version() ;
#endif
	if ( drv_ofs != 0 ) {
		_osmajor = drv_ofs ;
		_osminor = drv_ofs >> 8 ;
	}

	if ( _osmajor <= 2 ) {
		lastDrv = -1 ;
		return ;
	}

#if defined(__TURBOC__)
	_AH = 0x52 ;
	DOS_INT ;
	sysvars = (Bfptr)MK_FP(_ES,_BX) ;
#else
	sysvars = DOS_sysvars() ;
#endif

	if ( _osmajor == 3 && _osminor == 0 ) {
		cds_ofs = 0x17 ;
		drv_ofs = 0x1b ;
	} else {
		cds_ofs = 0x16 ;
		drv_ofs = 0x21 ;
	}

	lastDrv = sysvars[drv_ofs] ;
#if defined(__386__)
	cds = (Bfptr)( ((*(Wfptr)&sysvars[cds_ofs+2])<<4) +
			 *(Wfptr)&sysvars[cds_ofs]	    ) ;
#else
	cds = (Bfptr)MK_FP(*(Wfptr)&sysvars[cds_ofs+2],*(Wfptr)&sysvars[cds_ofs]) ;
#endif
	cdsSize = (_osmajor >= 4) ? 0x58 : 0x51 ;
}


static CDS FAR *currdir ( int drive )
{
	if ( cdsSize == 0 || drive >= lastDrv ) return (CDS FAR*)0 ;
	return (CDS FAR*)&cds[drive*cdsSize] ;
}


static int get_assign_list ( Word index, Bfptr local, Bfptr net,
			     Bptr pavail, Bptr pdevtype, Wptr puserval )
{
#if defined(__TURBOC__)
	Byte	avail, devtype	;
	Word	userval 	;

	asm push ds
	asm push di
	asm push si
	asm mov bx,index
	asm lds si,local
	asm les di,net
	asm mov ax,5f02h
	asm push bp		; /* destoryed BP ? */
	asm int 21h
	asm pop bp
	asm pop si
	asm pop di
	asm pop ds
	asm jc error
		asm mov byte ptr avail,bh
		asm mov byte ptr devtype,bl
		asm mov word ptr userval,cx
		*pavail   = avail ;
		*pdevtype = devtype ;
		*puserval = userval ;
		return 0 ;
error: ;
	return _AX ;	/* error no. */
#else
	long	x	;

	x = DOS_assignList(index,local,net) ;
	if ( x < 0 ) return x ;

	*pdevtype = (Byte)x ;
	*pavail   = ((Word)x >> 8) ;
	*puserval = x >> 16 ;
	return 0 ;
#endif
}

/******************************************************************************/

int get_all_drive ( char *drv )
{
	int	i, n, num		;
	Byte	local[128], net[128]	;
	Byte	avail, devType		;
	Word	userVal 		;
	CDS FAR *p			;

	memset(drvType,0,sizeof(drvType)) ;
	set_dos_env() ;

	num = 0 ;
	p = (CDS FAR*)cds ;
	for ( i = 0 ; i < lastDrv ; i++ ) {
		if ( p->flags & (NETWORK|PHYSICAL|JOIN|SUBST) ) {
			drvType[i] = 'A' + i ;
			num++ ;
		}
		p = (CDS FAR*)(((Byte FAR*)p)+cdsSize) ;
	}

	i = 0 ;
	while ( get_assign_list(i,local,net,&avail,&devType,&userVal) == 0 ) {
		if ( devType == 4 ) {	/* 4:disk drive, 3:printer */
			n = (local[0] & 0x1f) - 1 ;
/* ...			if ( n >= lastDrv && n < sizeof(drvType) &&	/**/
/* ...			     drvType[n] == 0			    ) { /**/
			if ( n < sizeof(drvType) ) {
				drvType[n] = i + 128 ;
				num++ ;
			}
		}
		i++ ;
	}

	if ( drv ) memcpy(drv,drvType,sizeof(drvType)) ;
	return num ;
}

/******************************************************************************/

#if defined(__386__)
static DPB FAR * fix386ptr ( void FAR * p )
{
	return (DPB FAR*)( (((long)p>>16)<<4) + (Word)p ) ;
}
#else
	#define fix386ptr(p)	p
#endif

#define maybe_ram_disk(p)	((p)->copies_fat == 1)

#if !defined(__BORLANDC__) && !defined(__WATCOMC__)
static Bfptr _fstrncpy ( Bfptr dest, Bfptr src, int maxlen )
{
	Bfptr	p	;

	p = dest ;
	while ( maxlen > 0 ) {
		if ( (*p=*src) == '\0' ) break ;
		src++ ;
		p++ ;
		maxlen-- ;
	}
	return dest ;
}


static Bfptr _fstrcpy ( Bfptr dest, Bfptr src )
{
	Bfptr	p	;

	p = dest ;
	while ( (*p=*src) != '\0' ) {
		src++ ;
		p++ ;
	}
	return dest ;
}
#endif


static int is_hard_disk ( DPB FAR *p )
{
	Byte	mType	;

	mType = ( _osmajor >= 4 ? p->vers.dos4.media_descriptor :
				  p->vers.dos3.media_descriptor ) ;

	return mType == 0xf8 ? 1 : 0 ;
}


int get_drive_info ( int drv, char info[] )
{
	CDS FAR *p		;
	Byte	avail, devType	;
	Word	userVal 	;
	Byte	local[16]	;

	if ( lastDrv == 0 ) get_all_drive(0) ;

	*(Wptr)&info[0] = 0 ;
	if ( (unsigned)drv >= sizeof(drvType) || drvType[drv] == 0 ) return -1 ;

	if ( drvType[drv] < 128 ) {	/* in DOS CDS[] */
		p = currdir(drv) ;
		if ( p == 0 ) return -1 ;

		if ( p->flags & JOIN ) {
			info[0] = 'J' ;                         /* JOIN       */
			_fstrcpy(&info[1],p->current_path) ;
		} else if ( p->flags & SUBST ) {
			info[0] = 'S' ;                         /* SUBST      */
			_fstrncpy(&info[1],p->current_path,p->backslash_offset) ;
			info[p->backslash_offset+1] = '\0';
		} else if ( p->flags & NETWORK ) {
			if ( (p->flags & REDIR_NOT_NET) &&
			     p->current_path[0] > 'Z'      ) {
				info[0] = 'C' ;                 /* CD-ROM     */
			} else {
				info[0] = 'N' ;                 /* NETWORK    */
				_fstrcpy(&info[1],p->current_path) ;
			}
		} else if ( maybe_ram_disk(fix386ptr(p->dpb)) ) { /* RAM-DISK ? */
			info[0] = 'R' ;                         /* RAM-DISK   */
		} else if ( is_hard_disk(fix386ptr(p->dpb)) ) {
			info[0] = 'H' ;                         /* HARD-DISK  */
		} else {
			info[0] = 'F' ;                         /* FLOPPY-DISK */
		}

		return 1 ;
	}

	info[0] = 'N' ;                                         /* NETWORK    */
	return get_assign_list(drvType[drv]&0x7f,local,(Bfptr)&info[1],
			       &avail,&devType,&userVal) == 0 ? 1 : -1 ;
}

/******************************************************************************/
#if defined(TEST)

#include <ctype.h>

static char * drv_Type ( char c )
{
	switch( c ) {
	    case 'H' : return "HARD-DISK" ;
	    case 'F' : return "FLOPPY" ;
	    case 'R' : return "RAM-DISK" ;
	    case 'J' : return "JOIN" ;
	    case 'S' : return "SUBST" ;
	    case 'C' : return "CD-ROM" ;
	    case 'N' : return "NETWORK" ;
	}
	return "?" ;
}


int main ( int argc, char *argv[] )
{
	char	drv[26] ;
	int	i, n	;
	char	s[128]	;

	if ( argc > 1 && isalpha(argv[1][0]) ) {
		i = (argv[1][0]&0x1f) - 1 ;
		if ( get_drive_info(i,s) < 0 ) return 0 ;
		printf("%c: %s",i+'A',drv_Type(s[0])) ;
		if ( s[1] ) printf(" -> %s",&s[1]) ;
		putchar('\n') ;
		return s[0] ;
	}

	printf("Total drives = %d\n",n=get_all_drive(drv)) ;

	for ( i = 0 ; i < sizeof(drv) ; i++ ) {
		if ( drv[i] == 0 ) continue ;

		if ( get_drive_info(i,s) < 0 ) {
			printf("%c: --> Read error !",i+'A') ;
		} else {
			printf("%c: %s",i+'A',drv_Type(s[0])) ;
			if ( s[1] ) printf(" -> %s",&s[1]) ;
		}
		putchar('\n') ;
	}
	return n ;
}

#endif
/******************************************************************************/
