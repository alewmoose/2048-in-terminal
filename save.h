#ifndef SAVE_H
#define SAVE_H
#include "board.h"

int load_game(board_t board, int *score, int *max_score);
int save_game(board_t board, int  score, int max_score);

#endif
