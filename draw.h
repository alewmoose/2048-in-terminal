#ifndef DRAW_H
#define DRAW_H

#include "common.h"

#define TILE_WIDTH  10
#define TILE_HEIGHT 5

#define WIN_OK        0
#define WIN_TOO_SMALL -1

/* Set up screen, keyboard, colors */
void setup_screen(void);

/* (Re-)initialize board and stats windows.
 * Returns WIN_OK or WIN_TOO_SMALL if terminal's too small.
 * exit(1) on error */
int  init_win(void);

/* Print the 'TERMINAL IS TOO SMALL' message */
void print_too_small(void);

/* Draw board and stats. Both can be omitted if NULL is passed */
void draw(const Board *board, const Stats *stats);

/* Draw sliding animation. 'moves' must hold distance (positive int) for
 * each sliding tile and 0 for static and empty tiles */
void draw_slide(const Board *board, const Board *moves, Dir dir);

#endif
