#ifndef DRAW_H
#define DRAW_H

#include "common.h"

#define TILE_WIDTH  10
#define TILE_HEIGHT 5

#define WIN_OK        0
#define WIN_TOO_SMALL -1


void setup_screen(void);
int  init_win();
void print_too_small(void);
void draw(const Board *board, const Stats *stats);
void draw_slide(Board *board, const Board *moves, Dir dir);


#endif
