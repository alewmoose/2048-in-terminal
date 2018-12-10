#ifndef BOARD_H
#define BOARD_H

#include "common.h"

#define NO_SLIDE -1

/* Clear board, add two '2' tiles */
void board_start(Board *board);

/* Add tile in random position.
 * If 'only2' is false, the tile may be '2' or '4' */
void board_add_tile(Board *board, bool only2);

/* Returns points, sets 'new_board' and 'moves'(needed for animation).
 * Returns NO_SLIDE if didn't slide */
int  board_slide(const Board *board, Board *new_board, Board *moves,  Dir dir);

bool board_can_slide(const Board *board);

#endif
