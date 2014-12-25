#ifndef SAVE_H
#define SAVE_H
#include "common.h"

int lock_save_file(void);
int load_game(Board *board, Stats *stats);
int save_game(const Board *board, const Stats *stats);

#endif
