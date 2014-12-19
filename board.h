#ifndef BOARD_H
#define BOARD_H

#include "common.h"

#define NO_SLIDE -1

void board_copy(board_t dest, board_t source);

void board_start    (board_t board);
void board_add_tile (board_t board, bool only2);
int  board_slide    (board_t board, board_t new_board, board_t moves,  dir_t dir);
bool board_can_slide(board_t board);

bool save_game(board_t board, int  score, int max_score);
bool load_game(board_t board, int *score, int *max_score);

#endif
