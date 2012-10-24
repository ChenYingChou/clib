/*
 * $Log: tbrowse.h $
 *
**/

#if !defined(_TBROWSE_H_)
#define _TBROWSE_H_

#include	"c0.h"
#include	"inkey.h"
#include	"twin.h"

class Tbrowse : public twin {
    private:
	long	_pos		;
	long	_total		;
	int	_screenHigh	;
	int	_visibleHigh	;
	Byte	_cNor		;
	Byte	_cBar		;

    protected:
	int	_nth		;

    public:
	Tbrowse ( int r1, int c1, int r2, int c2 ) ;

	int browse ( int bar, const char *title=0 ) ;
	void refresh ( int show ) ;

	void set_color ( Byte cNor, Byte cBar ) { _cNor = cNor ; _cBar = cBar ; }
	void get_color ( Bptr cNor, Bptr cBar ) { *cNor = _cNor ; *cBar = _cBar ; }

	int visible   ( ) { return _visibleHigh 			;}
	long position ( ) { return _pos 				;}
	long total    ( ) { return _total				;}
	void position ( long pos ) { _pos = pos 			;}
	void total    ( long tot ) { _total = tot			;}

	virtual int input ( int nth ) { return get_key()		;}
	virtual void showData ( int nth ) = 0 ;
	virtual void moveData ( int num ) { _pos += num 		;}

	virtual int initial ( const char *title )
	{
		_screenHigh = display_high() ;
		return 1 ;
	}

	virtual int getData ( int nth )
	{
		return (unsigned long)(_pos+nth) < _total ;	// true or false
	}
}	;

#endif	// _TBROWSE_H_
