CFLAGS=-Wall -Wextra -Wpedantic -std=c99 -g -O2

.PHONY: all install clean

all: 2048

2048: game.o board.o draw.o save.o logit.o
	cc game.o board.o draw.o save.o logit.o -o 2048 $(CFLAGS) -lncurses


game.o: game.c board.h draw.h common.h
	cc -c game.c $(CFLAGS)

board.o: board.c board.h common.h
	cc -c board.c $(CFLAGS)

draw.o: draw.c draw.h common.h

save.o: save.c save.h common.h
	cc -c save.c $(CFLAGS)

logit.o: logit.c logit.h
	cc -c logit.c $(CFLAGS)

install:
	cp 2048 /usr/local/bin


clean:
	rm -f 2048 *.o

