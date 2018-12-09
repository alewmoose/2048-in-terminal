TARGET=2048
SRC=$(wildcard *.c)
OBJ=$(SRC:.c=.o)
DEP=$(SRC:.c=.d)
CFLAGS=-Wall -Wextra -pedantic -g -std=c99 -O2 -march=native
LDLIBS=-lncurses
DESTDIR=/usr/local/bin

.PHONY: all install clean

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $^ -o $@ $(LDLIBS)


%.o : %.c

%.o: %.c %.d
	$(CC) -c $(CFLAGS) $< -o $@

%.d: %.c
	$(CC) -MM $< > $@


.PRECIOUS: $(DEP)

include $(DEP)


install:
	@-install -m 755 $(TARGET) $(DESTDIR)

clean:
	@-rm -f $(TARGET) $(OBJ) $(DEP)
