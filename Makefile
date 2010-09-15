all: demo1 demo2 demo3 

demo1: demo1.c jitlib-core.o
	gcc -c -g -Winline -Wall -std=c99 -pedantic -D_XOPEN_SOURCE=600 demo1.c
	gcc -o demo1 -g -Wall -std=c99 -pedantic demo1.o jitlib-core.o -lopcodes

demo2: demo2.c jitlib-core.o
	gcc -c -g -Winline -Wall -std=c99 -pedantic -D_XOPEN_SOURCE=600 demo2.c
	gcc -o demo2 -g -Wall -std=c99 -pedantic demo2.o jitlib-core.o -lopcodes

demo3: demo3.c jitlib-core.o
	gcc -O2 -c -g -Winline -Wall -std=c99 -pedantic -D_XOPEN_SOURCE=600 demo3.c
	gcc -O2 -o demo3 -g -Wall -std=c99 -pedantic demo3.o jitlib-core.o -lopcodes

debug: debug.c jitlib-core.o
	gcc -c -g -Winline -Wall -std=c99 -pedantic -D_XOPEN_SOURCE=600 debug.c
	gcc -o debug -g -Wall -std=c99 -pedantic debug.o jitlib-core.o -lopcodes


jitlib-core.o: myjit/jitlib.h myjit/jitlib-core.h myjit/jitlib-core.c myjit/jitlib-debug.c myjit/x86-codegen.h myjit/x86-specific.h myjit/reg-allocator.h myjit/flow-analysis.h myjit/set.h myjit/amd64-specific.h myjit/amd64-codegen.h myjit/sparc-codegen.h myjit/sparc-specific.h myjit/llrb.c myjit/reg-allocator.h myjit/rmap.h myjit/cpu-detect.h myjit/x86-common-stuff.c
	gcc -c -g -Winline -Wall -std=c99 -pedantic -D_XOPEN_SOURCE=600 myjit/jitlib-core.c

clean:
	rm -f demo1
	rm -f demo2
	rm -f demo3
	rm -f debug
	rm -f *.o
