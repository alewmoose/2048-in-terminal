#ifndef BOARD_H
#define BOARD_H

#include "common.h"

#define NO_SLIDE -1

void board_start    (Board *board);
void board_add_tile (Board *board, bool only2);
int  board_slide    (const Board *board, Board *new_board, Board *moves,  Dir dir);
bool board_can_slide(const Board *board);

#endif
