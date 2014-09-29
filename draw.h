#ifndef DRAW_H
#define DRAW_H

#define SW 10   // square width and height
#define SH 5

#define VL  ACS_VLINE
#define HL  ACS_HLINE
#define ULC ACS_ULCORNER
#define URC ACS_URCORNER
#define LLC ACS_LLCORNER
#define LRC ACS_LRCORNER

void init_win(WINDOW **win, WINDOW **score_win);
void draw_field(WINDOW *win, int field[5][5], int gameover);
void draw_score(WINDOW *score_win, int points, int score, int max_score);
void animove(WINDOW *win, int field[5][5], int moves[5][5], int dir);
void sleep_sec(double sec);
void prepare_screen(void);


#endif
