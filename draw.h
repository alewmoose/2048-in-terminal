#ifndef DRAW_H
#define DRAW_H

#include "common.h"

#define TILE_WIDTH  10
#define TILE_HEIGHT 5

#define WIN_OK        1
#define WIN_TOO_SMALL 0


void setup_screen(void);
bool init_win    (WINDOW **board_win, WINDOW **score_win);

void print_too_small(void);

void draw_slide(WINDOW *win, board_t board, board_t moves, dir_t dir);

void refresh_board(WINDOW *board_win, board_t board, bool is_gameover);
void refresh_score(WINDOW *score_win, int score, int points, int max_score);


#endif
