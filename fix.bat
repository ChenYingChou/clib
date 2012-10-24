@echo off
if [%1]==[]	goto finish

set objx=
set objs=
set libs=
:loop
if [%1]==[]	goto start
set objs=%objs% %1.obj
set libs=%libs% -+%1.obj
set objx=%objx% %1.o
shift
goto loop

:start

del *.obj

call setbc
make -DMODULE=s %objs%
tlib bstwin.lib %libs%
tlib bstwinx.lib %libs%
del *.obj
make -DMODULE=l %objs%
tlib bltwin.lib %libs%
tlib bltwinx.lib %libs%
del *.obj
call nobc
:::: goto skipWC

call setwc
make -DMODULE=s %objs%
wlib wstwin.lib %libs%
wlib wstwinx.lib %libs%
del *.obj
make -DMODULE=l %objs%
wlib wltwin.lib %libs%
wlib wltwinx.lib %libs%
del *.obj
make -DWC386=1 %objs%
wlib wftwin.lib %libs%
wlib wftwinx.lib %libs%
del *.obj
call nowc

:skipWC
call setgcc
make %objx%
ar rs djtwin.a %objx%
ar rs djtwinx.a %objx%
del *.o
call nogcc

:finish
set objx=
set objs=
set libs=
