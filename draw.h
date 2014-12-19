#ifndef DRAW_H
#define DRAW_H

#define TILE_WIDTH  10
#define TILE_HEIGHT 5

#define WIN_OK 1
#define WIN_TOO_SMALL 0


typedef int board_t[4][4];

void setup_screen(void);
int  init_win    (WINDOW **board_win, WINDOW **score_win);

void print_too_small(void);

void draw_board  (WINDOW *board_win, board_t board, int is_gameover);
void draw_score  (WINDOW *score_win, int score, int points, int max_score);
void draw_slide  (WINDOW *win, board_t board, board_t moves, int dir);

void refresh_board(WINDOW *board_win, board_t board, int is_gameover);
void refresh_score(WINDOW *score_win, int score, int points, int max_score);


#endif
