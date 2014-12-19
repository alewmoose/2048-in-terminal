CFLAGS=-Wall -Wextra -Wpedantic -std=c99 -g -O2

2048: game.o board.o draw.o
	cc game.o board.o draw.o -o 2048 $(CFLAGS) -lncurses

game.o: game.c board.h draw.h
	cc -c game.c $(CFLAGS)

board.o: board.c board.h
	cc -c board.c $(CFLAGS)

draw.o: draw.c draw.h
	cc -c draw.c $(CFLAGS)

clean:
	rm -f 2048 *.o

