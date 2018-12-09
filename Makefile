EXE=2048-in-terminal
SRC=$(wildcard *.c)
OBJ=$(SRC:.c=.o)
DEP=$(SRC:.c=.d)
CFLAGS=-Wall -Wextra -pedantic -g -std=c99 -O2 -march=native
LDLIBS=-lncurses
PREFIX=/usr/local
BINDIR=$(PREFIX)/bin

.PHONY: all install clean

all: $(EXE)

$(EXE): $(OBJ)
	$(CC) $(CFLAGS) $^ -o $@ $(LDLIBS)


%.o : %.c

%.o: %.c %.d
	$(CC) -c $(CFLAGS) $< -o $@

%.d: %.c
	$(CC) -MM $< > $@


.PRECIOUS: $(DEP)

include $(DEP)


install:
	@-install -m 755 $(EXE) $(BINDIR)

clean:
	@-rm -f $(EXE) $(OBJ) $(DEP)
