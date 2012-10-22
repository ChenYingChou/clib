/*
 * $Log: TWIN2.CPP $
 * Revision 1.0  1996-02-06 23:29:13-0800  YCCHEN
 * Initial revision
 *

Wfptr twin::save_screen( )						;
void twin::restore_screen( Wfptr screen, int dofree )			;
void twin::free_image( )						;
Wptr twin::save_image( )						;
void twin::restore_image( int dofree )					;

*/

#include	<string.h>
#include	<malloc.h>

#define 	_TWIN_
#include	"twin.h"

#if defined(__WATCOMC__)

    #if defined(__386__)
	#define MALLOC			malloc
	#define FREE			free
    #else
	#define MALLOC			_fmalloc
	#define FREE			_ffree
    #endif

#elif defined(__DJGPP__)

	#include <go32.h>
	#include <sys/nearptr.h>

	#define _fmemmove(dest,src,cnt) memmove(dest,src,cnt)
	#define MALLOC			malloc
	#define FREE			free

#else

	#define MALLOC			farmalloc
	#define FREE			farfree

#endif

static void video_to_buffer ( Wfptr pv, int W, int w, int h, Wfptr pb )
{
#if defined(__DJGPP__)
	DJNearptr_enable() ;
	pv = (Wfptr)((Bfptr)pv + __djgpp_conventional_base) ;
#endif
	if ( W == w ) { /* continuous blocks */
		_fmemmove(pb,pv,2*W*h) ;
	} else {	/* move row by row */
		while ( h > 0 ) {
			_fmemmove(pb,pv,2*w) ;
			pv += W ;
			pb += w ;
			h-- ;
		}
	}
#if defined(__DJGPP__)
	DJNearptr_disable() ;
#endif
}


static void buffer_to_video ( Wfptr pv, int W, int w, int h, Wfptr pb )
{
#if defined(__DJGPP__)
	DJNearptr_enable() ;
	pv = (Wfptr)((Bfptr)pv + __djgpp_conventional_base) ;
#endif
	if ( W == w ) { /* continuous blocks */
		_fmemmove(pv,pb,2*W*h) ;
	} else {	/* move row by row */
		while ( h > 0 ) {
			_fmemmove(pv,pb,2*w) ;
			pv += W ;
			pb += w ;
			h-- ;
		}
	}
#if defined(__DJGPP__)
	DJNearptr_disable() ;
#endif
}


Wfptr twin::save_screen ( )
{
	int	w = _C2_ - _C1_ + 1 ;
	int	h = _R2_ - _R1_ + 1 ;
	Wfptr	p = (Wfptr)MALLOC(w*h*2)	;

	if ( p == 0 ) return 0 ;	/* cannot save screen */
	video_to_buffer(videoPtr+(_R1_*videoWidth+_C1_),videoWidth,w,h,p) ;
	return p ;
}


void twin::restore_screen ( Wfptr screen, int dofree )
{
	if ( screen != 0 ) {
		buffer_to_video(videoPtr+(_R1_*videoWidth+_C1_),videoWidth,
				_C2_-_C1_+1,_R2_-_R1_+1,screen) ;
		if ( dofree ) FREE(screen) ;
	}
}


void twin::free_image ( )
{
	if ( pBuffer_ ) {
		delete[] pBuffer_ ;
		pBuffer_ = 0 ;
	}
	oMode_ &= ~oToBuffer_ ;
	if ( (oMode_ & (oToVideo_|oToBios_)) == 0 ) {
		if ( directvideo )
			set_toVideo(1) ;
		else
			set_toBios(1) ;
	}
}


Wptr twin::save_image ( )
{
	int	w = _C2_ - _C1_ + 1 ;
	int	h = _R2_ - _R1_ + 1 ;

	if ( pBuffer_ == 0 ) {
		pBuffer_ = new Word[w*h] ;
		if ( pBuffer_ == 0 ) return 0 ; /* cannot save image */
	}

	video_to_buffer(videoPtr+(_R1_*videoWidth+_C1_),videoWidth,w,h,pBuffer_) ;
	return pBuffer_ ;
}


void twin::restore_image ( int dofree )
{
	if ( pBuffer_ != 0 ) {
		buffer_to_video(videoPtr+(_R1_*videoWidth+_C1_),videoWidth,
				_C2_-_C1_+1,_R2_-_R1_+1,pBuffer_) ;
		if ( dofree ) free_image() ;
	}
}

