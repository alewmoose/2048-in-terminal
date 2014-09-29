#define _GNU_SOURCE
#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "draw.h"

static const char *centered_nums[] = { "        ",
	"   2    ", "   4    ", "   8    ", "   16   ",
	"   32   ", "   64   ", "  128   ", "  256   ",			    
	"  512   ", "  1024  ", "  2048  ", "  4096  ",			    
	"  8192  ", " 16384  ", " 32768  ", " 65536  ",			    
	" 131072 "
};

static const struct timespec tick = {.tv_sec = 0, .tv_nsec = 5000000};
static const struct timespec endmove = {.tv_sec = 0, .tv_nsec = 6000000};

void init_win(WINDOW **win, WINDOW **score_win)
{
	if (*win) delwin(*win);
	if (*score_win) delwin(*score_win);
	clear();
	refresh();

	int scr_width, scr_height;
	getmaxyx(stdscr, scr_height, scr_width);

	int win_width = SW*4 + 2;
	int win_height = SH*4 + 2;
	int win_top = (scr_height - win_height) / 2;
	int win_left = (scr_width - win_width) / 2 -5;

	*win = newwin(win_height, win_width, win_top, win_left);
	wborder(*win, VL, VL, HL, HL, ULC, URC, LLC, LRC);
	keypad(*win, TRUE);

	*score_win = newwin(win_height-2, 13, win_top+1, win_left+win_width+1);
	
	if (!win || !score_win) {
		endwin();
		exit(1);
	}
}

void prepare_screen(void)
{
	initscr();
	start_color();
    noecho(); 
    cbreak();
    curs_set(0);

	init_pair(1, COLOR_WHITE,   COLOR_BLACK);
	init_pair(2, COLOR_YELLOW,  COLOR_BLACK);
	init_pair(3, COLOR_GREEN,   COLOR_BLACK);
	init_pair(4, COLOR_BLUE,    COLOR_BLACK);
	init_pair(5, COLOR_MAGENTA, COLOR_BLACK);
	init_pair(6, COLOR_CYAN,    COLOR_BLACK);
	init_pair(7, COLOR_RED,     COLOR_BLACK);
}

static void square_prop(int num, int *centered, int *color)
{
	switch (num) {
		case 2:      *centered = 1;  *color = 1; break;
		case 4:      *centered = 2;  *color = 2; break;
		case 8:      *centered = 3;  *color = 3; break;
		case 16:     *centered = 4;  *color = 4; break;
		case 32:     *centered = 5;  *color = 5; break;
		case 64:     *centered = 6;  *color = 6; break;
		case 128:    *centered = 7;  *color = 7; break;
		case 256:    *centered = 8;  *color = 1; break;
		case 512:    *centered = 9;  *color = 2; break;
		case 1024:   *centered = 10; *color = 3; break;
		case 2048:   *centered = 11; *color = 4; break;
		case 4096:   *centered = 12; *color = 5; break;
		case 8192:   *centered = 14; *color = 6; break;
		case 16384:  *centered = 15; *color = 7; break;
		case 32768:  *centered = 16; *color = 1; break;
		case 65536:  *centered = 17; *color = 2; break;
		case 131072: *centered = 17; *color = 3; break;
		default:     *centered = 0;  *color = 0;
	}	
}



void draw_square(WINDOW *win, int y, int x, int num)
{
	int left, right, top, bottom;
	left = x;
	right = left + SW-1;
	top = y;
	bottom = top + SH-1;
              // draw empty square
	if (num == 0) {
		for (int y = top; y <= bottom; y++) {
			mvwprintw(win, y, left, "          "); // 10 spaces
		}
		return;
	}
	int color, centered_i;
	square_prop(num, &centered_i, &color);
	wattron(win, COLOR_PAIR(color));

	if (num >= 256)   wattron(win, A_BOLD);
	if (num >= 32768) wattron(win, A_UNDERLINE);

	mvwaddch(win, top, left, ULC); // top line
	for (int x = left+1; x < right; x++) waddch(win, HL);
	waddch(win, URC);


	for (int y = top+1; y < bottom; y++) {
		mvwaddch(win, y, left, VL);
		if (y == (top + bottom) / 2) {  //central line with number
			const char *num_str = centered_nums[centered_i];
			wprintw(win, num_str);
		} else {
			wprintw(win, "        "); // 8 spaces
		}
		waddch(win, VL); 
	}

	mvwaddch(win, bottom, left, LLC); //bottom line
	for (int x = left+1; x < right; x++) waddch(win, HL);
	waddch(win, LRC);

	if (num >= 256)   wattroff(win, A_BOLD);
	if (num >= 32768) wattroff(win, A_UNDERLINE);
}



void draw_field(WINDOW *win, int field[5][5], int gameover)
{
	for (int y = 1; y <= 4; y++) {
		for (int x = 1; x <= 4; x++) {
			int yc, xc;
			xc = SW*(x-1) + 1;
			yc = SH*(y-1) + 1;
			draw_square(win, yc, xc, field[y][x]);
		}
	}
	if (gameover) {
		wattron(win, A_BOLD | COLOR_PAIR(1));	
		mvwprintw(win, SH*2, (SW*4-8)/2,  "GAME OVER");
		wattroff(win, A_BOLD);
	}
}

void draw_score(WINDOW *score_win, int points, int score, int max_score)
{
	wattron(score_win, COLOR_PAIR(2));
	mvwprintw(score_win, 1, 1, "Score");
	mvwprintw(score_win, 4, 1, "Best Score");

	if (points > 0) {
		wattron(score_win, COLOR_PAIR(3));
		mvwprintw(score_win, 1, 7, "%+6d", points);
	} else {
		mvwprintw(score_win, 1, 7, "       ");
	}

	wattron(score_win, COLOR_PAIR(1));
	mvwprintw(score_win, 2, 1, "%8d", score);
	mvwprintw(score_win, 5, 1, "%8d", max_score);

	wattron(score_win, COLOR_PAIR(3));
	mvwaddch(score_win, 15, 1, 'R');
	wattron(score_win, COLOR_PAIR(7));
	mvwaddch(score_win, 16, 1, 'Q');
	wattron(score_win, COLOR_PAIR(1));
	mvwprintw(score_win, 15, 2, "estart");
	mvwprintw(score_win, 16, 2, "uit");
}

typedef struct msq { //moving square
	int x, y;
	int tick;
	int num;
} msq_t;

static int sort_left(const void *l, const void *r)
{
	return ((msq_t *)l)->x - ((msq_t *)r)->x;
}

static int sort_right(const void *l, const void *r)
{
	return ((msq_t *)r)->x - ((msq_t *)l)->x;
}

static int sort_up(const void *l, const void *r)
{
	return ((msq_t *)l)->y - ((msq_t *)r)->y;
}

static int sort_down(const void *l, const void *r)
{
	return ((msq_t *)r)->y - ((msq_t *)l)->y;
}
				
void animove(WINDOW *win, int field[5][5], int moves[5][5], int dir)
{
	msq_t msq[16]; // moving squares
	int msq_n = 0;
	for (int y = 1; y <= 4; y++) {
		for (int x = 1; x <= 4; x++) {
			if (moves[y][x]) {
				msq[msq_n++] = (msq_t){.x = (x-1)*SW+1, .y = (y-1)*SH+1,
									.num = field[y][x], .tick = 6 / moves[y][x]};
				field[y][x] = 0;
			}
		}
	}

	int (*sort)(const void*, const void*);
	int mx = 0, my = 0; //coord modifiers
	switch (dir) {
		case 'l': sort = sort_left;  mx = -2; break; // width is two times shorter then height
		case 'r': sort = sort_right; mx = 2;  break;
		case 'u': sort = sort_up;    my = -1; break;
		case 'd': sort = sort_down;  my = 1;  break;
		default : exit(1); break;
	}
	qsort(msq, msq_n, sizeof(msq_t), sort);
	
	nanosleep(&tick, NULL);
	for (int t = 1; t <= 30; t++) { // 30 ticks
		for (int s = 0; s < msq_n; s++) {
			msq_t *square = &msq[s];
			if (t % square->tick == 0) { //time to move!
				square->x += mx;
				square->y += my;
			}
		}
		draw_field(win, field, 0);
		for (int s = 0; s < msq_n; s++) { // draw moving squares
			msq_t *square = &msq[s];
			draw_square(win, square->y, square->x, square->num);
		}
		wrefresh(win);
		nanosleep(&tick, NULL);
	}
	nanosleep(&endmove, NULL);
}

