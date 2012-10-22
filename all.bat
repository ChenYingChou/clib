@echo off
del *.obj

call setbc
make -DMODULE=s
del *.obj
make -DMODULE=l
del *.obj
call nobc

call setwc
make -DMODULE=s
del *.obj
make -DMODULE=l
del *.obj
make -DWC386=1
del *.obj
call nowc

call setgcc
make
del *.o
call nogcc

