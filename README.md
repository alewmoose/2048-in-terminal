# 2048-in-terminal

Terminal-based clone of [the 2048 game](https://play2048.co/)

![screen](screen.gif)

----

## Requirements
- GNU Make
- C compiler (GCC or Clang)
- pkg-config
- ncurses library
- ncurses development files

----

## Build

Build using default parameters:

`make`

Or build with parameters, e.g.:

`make CC=clang NCURSES_LIB=ncursesw EXE=2048`

----

## Install

Install to default location (`/usr/local/bin/`):

`sudo make install`

Alter the `PREFIX` parameter to install to `<PREFIX>/bin`, e.g.:

`make install PREFIX=~/.local`

## Uninstall

Run `make uninstall`. Specify `PREFIX` if you did so during installation.

