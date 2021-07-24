@ECHO OFF
setlocal EnableDelayedExpansion

gcc -c decode.c 

gcc -c itab.c 

gcc -c syn.c 

gcc -c syn-att.c 

gcc -c syn-intel.c 

gcc -c udis86.c 