CFLAGS=-Wall -std=c99 -g -O2

2048: 2048.o field.o draw.o -lncurses

clean:
	rm 2048 *.o

