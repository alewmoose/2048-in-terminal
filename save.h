#ifndef SAVE_H
#define SAVE_H

#include "common.h"

/* load_game and save_game return 0 on success  and -1 on failure.
 * If several instances of the game are running, only the first
 * one will write the save file */
int load_game(Board *board, Stats *stats);
int save_game(const Board *board, const Stats *stats);

#endif
