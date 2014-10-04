CFLAGS=-Wall -std=c99 -g -O2

2048: game.o board.o draw.o
	cc game.o board.o draw.o -o 2048 $(CFLAGS) -lncurses

clean:
	rm -f 2048 *.o

