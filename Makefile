TARGET:= 2048
SRC:= $(wildcard *.c)
OBJ:= $(SRC:.c=.o)
DESTDIR:= /usr/local
CFLAGS+= -Wall -Wextra -Wpedantic -std=c99 -O2 -g
LDFLAGS+= -lncurses

.PHONY: all install clean

all: $(TARGET)

2048: $(OBJ)
	$(CC) $(OBJ) -o $(TARGET) $(CFLAGS) $(LDFLAGS)


install:
	@- install -m 755 $(TARGET) $(DESTDIR)/bin

clean:
	@-rm -f $(TARGET) $(OBJ)

game.o: board.h draw.h save.h common.h
board.o: board.h common.h
draw.o: draw.h common.h
save.o: save.h common.h
