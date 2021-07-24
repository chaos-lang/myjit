@ECHO OFF
setlocal EnableDelayedExpansion

SET cc=gcc
SET CFLAGS = -g -std=c99 -Wall -pedantic -D_XOPEN_SOURCE=600 -O2

%cc% %CFLAGS% -c myjit-disasm.c

%cc% %CFLAGS% -c io.c

CD arm32
CALL make.bat
CD ..

CD sparc
CALL make.bat
CD ..

CD udis86
CALL make.bat
CD ..

%cc% %CFLAGS% -o myjit-disasm udis86/decode.o udis86/itab.o udis86/syn.o udis86/syn-att.o udis86/syn-intel.o udis86/udis86.o sparc/sparc-dis.o arm32/arm32-dis.o myjit-disasm.o io.o