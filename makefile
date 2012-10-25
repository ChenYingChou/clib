#
# $Log: MAKEFILE $
#

!if !$d(CLIB)
CLIB = .
!endif

!include "..\make.mif"

!if !$d(DJGPP)
DOSDRV = dosdrv
DOSDRV_O = dosdrv.$(O)
!endif
############################################################################
newLib		: $(LIBA) $(LIBX)
all		: dblist test t1 t2 t3 $(DOSDRV)
dblist		: dblist.exe
test		: test.exe
testhrt		: testhrt.exe
t1		: t1.exe
t2		: t2.exe t2.msg
t3		: t3.exe news.msg
dosdrv		: dosdrv.exe
############################################################################
OBJ1 = twin.$(O) twin1.$(O) twin2.$(O) twin3.$(O) twin4.$(O) twin5.$(O) \
       twinget.$(O) inkey.$(O) scrimg.$(O) dbase.$(O) dbasec.$(O) dbasef.$(O) \
       dbpack.$(O) dbinsert.$(O) tbrowse.$(O) comm.$(O)
OBJ2 = hrtimer.$(O) chinese.$(O) doall.$(O) irq.$(O) commdrv.$(O) \
       commirq.$(O) cmos.$(O) kbd.$(O) timer.$(O) $(DOSDRV_O) \
       taskcom.$(O) task.$(O) debug.$(O)

$(LIBA) 	: $(OBJ1) $(OBJ2) irqwrap.$(O)
!if $d(DJGPP)
	&$(LIB) $< {$? }
!else
	&$(LIB) $< {-+$(?:.obj=) }
	@if exist $:$&.bak  erase $:$&.bak
!endif

$(LIBX) 	: $(LIBA) zfortify.$(O) fortify.$(O)
	@copy $(LIBA) $(LIBX)
!if $d(DJGPP)
	$(LIB) $< zfortify.$(O) fortify.$(O)
!else
	&$(LIB) $< +zfortify.$(O) +fortify.$(O)
	@if exist $:$&.bak  erase $:$&.bak
!endif

chinese.$(O)	: chinese.c chinese.h
hrtimer.$(O)	: hrtimer.c hrtimer.h
doall.$(O)	: doall.c doall.h
twin.$(O)	: twin.cpp twin.h
twin1.$(O)	: twin1.cpp twin.h
twin2.$(O)	: twin2.cpp twin.h
twin3.$(O)	: twin3.cpp twin.h
twin4.$(O)	: twin4.cpp twin.h
twinget.$(O)	: twinget.cpp twin.h inkey.h chinese.h
dbase.$(O)	: dbase.cpp dbase.h dbase0.h
dbasec.$(O)	: dbasec.cpp dbase.h dbase0.h
dbpack.$(O)	: dbpack.cpp dbase.h dbase0.h
dbinsert.$(O)	: dbinsert.cpp dbase.h dbase0.h
dbasef.$(O)	: dbasef.cpp dbase.h
scrimg.$(O)	: scrimg.cpp twin.h inkey.h scrlay.h
cmos.$(O)	: pchw.h
irq.$(O)	: pchw.h
kbd.$(O)	: pchw.h
timer.$(O)	: pchw.h
commirq.$(O)	: pchw.h
commdrv.$(O)	: pchw.h comm.h
fortify.$(O)	: fortify.c fortify.h ufortify.h
	$(CC) -DFORTIFY $&.c
zfortify.$(O)	: zfortify.cpp zfortify.hpp ufortify.hpp
	$(CCP) -DZFORTIFY $&.cpp
debug.$(O)	: debug.c debug.h
	$(CC) -DDEBUG $&.c
############################################################################
taskcom.$(O)	: taskcom.c task0.h task.h inkey.h pchw.h comm.h
!if $d(WATCOM) && !$d(WC386)
	$(CC) -4 $&.c
!endif

task.$(O)	: task.c task0.h task.h
!if $d(DJGPP)
	$(CC) -fomit-frame-pointer $&.c
!elif $d(BC)
	$(CC) -3 $&.c
!elif $d(WATCOM) && !$d(WC386)
	$(CC) -4 $&.c
!endif
############################################################################
dblist.exe	: dblist.$(O)
	$(LINK) $** $(LIBA)
dblist.$(O)     : dblist.cpp dbase.h
############################################################################
test.exe	: test.$(O)
	$(LINK) $** $(LIBA)
test.$(O)     : test.cpp twin.h inkey.h
############################################################################
testhrt.exe	: testhrt.$(O)
	$(LINK) $** $(LIBA)
testhrt.$(O)     : testhrt.c HRTimer.h
############################################################################
t1.exe		: t1.$(O)
	$(LINK) $** $(LIBA)
t1.$(O)       : t1.cpp twin.h inkey.h
############################################################################
t2.exe		: t2.$(O)
	$(LINK) $** $(LIBA)
t2.$(O) 	: t2.cpp twin.h inkey.h scrlay.h
t2.msg		: t2.scr
	scrgen -o$< $**
############################################################################
t3.exe		: t3.$(O)
	$(LINK) $** $(LIBA)
t3.$(O) 	: t3.cpp twin.h inkey.h scrlay.h
news.msg	: news.scr
	scrgen -o$< $**
############################################################################
dosdrv.exe	: dosdrvo.$(O)
	$(LINK) $**
dosdrvo.$(O)	: dosdrv.c
	$(CC) -DTEST $(OO) $**
############################################################################
clean		:
	@if exist *.$(O) erase *.$(O)
	@if exist *.exe  erase *.exe
	@if exist *.msg  erase *.msg
#	@if exist *.$(A) erase *.$(A)
############################################################################

