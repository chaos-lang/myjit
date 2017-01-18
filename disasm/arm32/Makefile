all: arm32-dis.o 

#CFLAGS = -g -std=c99 -Wall -pedantic -D_XOPEN_SOURCE=600

arm32-dis.o: arm32-dis.c arm32-dis.h
	$(CC) $(CFLAGS) -c arm32-dis.c 

clean:
	rm -f *.o
