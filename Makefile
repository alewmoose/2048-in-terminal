TARGET= 2048
SRC= $(wildcard *.c)
OBJ= $(SRC:.c=.o)
CC?= gcc
CFLAGS= -Wall -Wextra -pedantic -g -std=c99 -O2 -march=native
LDLIBS= -lncurses
DESTDIR= /usr/local

.PHONY: all install clean

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET) $(LDLIBS) 


install:
	@-install -m 755 $(TARGET) $(DESTDIR)/bin

clean:
	@-rm -f $(TARGET) $(OBJ)

game.o: board.h draw.h save.h common.h
board.o: board.h common.h
draw.o: draw.h common.h
save.o: save.h common.h
