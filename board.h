#ifndef BOARD_H
#define BOARD_H

#ifndef BOARD_T
#define BOARD_T
typedef int board_t[4][4];
#endif

void board_copy(board_t dest, board_t source);

void board_start    (board_t board);
void board_add_tile (board_t board, int only2);
int  board_slide    (board_t board, board_t new_board, board_t moves,  int dir);
int  board_can_slide(board_t board);

int save_game(board_t board, int score, int max_score);
int load_game(board_t board, int *score, int *max_score);

#endif
