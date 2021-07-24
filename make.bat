@ECHO OFF
setlocal EnableDelayedExpansion

SET compiler=gcc

IF [%1]==[] (
    CALL :Build
) ELSE IF [%1]==[clang] (
    SET compiler=clang
    CALL :Build
) ELSE IF [%1]==[demo1] (
    %compiler% -c -g -Winline -Wall -std=c99 -pedantic -D_XOPEN_SOURCE=600 demo1.c
    IF errorlevel 1 (
        EXIT /B 1
    )
	%compiler% -o demo1 -g -Wall -std=c99 -pedantic demo1.o jitlib-core.o
) ELSE IF [%1]==[myjit-disassembler] (
    CD disasm
    CALL make.bat
    CD ..
    XCOPY /K /D /H /Y disasm\myjit-disasm.exe .
)
EXIT /B 0

:Build
%compiler% -c -g -Winline -Wall -std=c99 -pedantic -D_XOPEN_SOURCE=600 myjit/jitlib-core.c
IF errorlevel 1 (
    EXIT /B 1
)
EXIT /B 0