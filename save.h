#ifndef SAVE_H
#define SAVE_H
#include "common.h"

int load_game(Board *board, Stats *stats);
int save_game(const Board *board, const Stats *stats);

#endif
