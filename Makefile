EXE?=2048-in-terminal
TARGET=$(BUILDDIR)/$(EXE)

SRCDIR=src
BUILDDIR=_build
$(shell mkdir -p $(BUILDDIR))

SRC=$(wildcard $(SRCDIR)/*.c)
OBJ=$(SRC:$(SRCDIR)/%.c=$(BUILDDIR)/%.o)
DEP=$(SRC:$(SRCDIR)/%.c=$(BUILDDIR)/%.d)

NCURSES_LIB?=ncurses
NCURSES_CFLAGS?=$(shell pkg-config --cflags $(NCURSES_LIB))
NCURSES_LDLIBS?=$(shell pkg-config --libs $(NCURSES_LIB))

CFLAGS+= -Wall -Wextra -pedantic -std=c11 -D_GNU_SOURCE $(NCURSES_CFLAGS)
LDLIBS+= $(NCURSES_LDLIBS)

PREFIX?=/usr/local
BINDIR?=$(PREFIX)/bin


.PHONY: all clean install uninstall

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) $^ -o $(TARGET) $(LDLIBS)

%.o : %.c

$(BUILDDIR)/%.o: $(SRCDIR)/%.c $(BUILDDIR)/%.d
	$(CC) -MM $< -MQ $(BUILDDIR)/$*.o -MF $(BUILDDIR)/$*.d
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $< -o $@

%.d: ;

-include $(DEP)


clean:
	@-rm -r $(BUILDDIR)

install: $(TARGET)
	install -m 755 $(TARGET) $(BINDIR)

uninstall:
	rm $(BINDIR)/$(EXE)

