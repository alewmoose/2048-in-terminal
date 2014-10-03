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

static const struct timespec tick_time     = {.tv_sec = 0, .tv_nsec = 5000000};
static const struct timespec end_move_time = {.tv_sec = 0, .tv_nsec = 6000000};



int init_win(WINDOW **board_win, WINDOW **score_win)
{
	if (*board_win) delwin(*board_win);
	if (*score_win) delwin(*score_win);
	clear();
	refresh();

	int scr_width, scr_height;
	getmaxyx(stdscr, scr_height, scr_width);

	int board_win_width  = TILE_WIDTH*4  + 2;
	int board_win_height = TILE_HEIGHT*4 + 2;
	int board_win_top    = (scr_height - board_win_height) / 2;
	int board_win_left   = (scr_width - board_win_width)   / 2 - 5;

	*board_win = newwin(board_win_height, board_win_width,
	                    board_win_top, board_win_left);

	wborder(*board_win, ACS_VLINE, ACS_VLINE, ACS_HLINE, ACS_HLINE,
	         ACS_ULCORNER, ACS_URCORNER, ACS_LLCORNER, ACS_LRCORNER);

	keypad(*board_win, TRUE);

	*score_win = newwin(board_win_height - 2, 13, board_win_top+1,
	                    board_win_left + board_win_width + 1);
	
	if (!board_win || !score_win) return 0;
	return 1;
}

void setup_screen(void)
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



static void draw_tile(WINDOW *board_win, int top, int left, int val)
{
//	return;
	int right  = left + TILE_WIDTH  - 1;
	int bottom = top  + TILE_HEIGHT - 1;

              // draw empty square
	if (val == 0) {
		for (int y = top; y <= bottom; y++) {
			mvwprintw(board_win, y, left, "          "); // 10 spaces
		}
		return;
	}

	int color, centered_i;
	square_prop(val, &centered_i, &color);
	wattron(board_win, COLOR_PAIR(color));
	if (val >= 256)   wattron(board_win, A_BOLD);
	if (val >= 32768) wattron(board_win, A_UNDERLINE); // it's possible :)

	// draw top line
	mvwaddch(board_win, top, left, ACS_ULCORNER);
	for (int x = left+1; x < right; x++) waddch(board_win, ACS_HLINE);
	waddch(board_win, ACS_URCORNER);

	for (int y = top+1; y < bottom; y++) {
		mvwaddch(board_win, y, left, ACS_VLINE);
		if (y == (top + bottom) / 2) {  //central line with number
			const char *num_str = centered_nums[centered_i];
			wprintw(board_win, num_str);
		} else {
			wprintw(board_win, "        "); // 8 spaces
		}
		waddch(board_win, ACS_VLINE); 
	}

	// draw bottom line
	mvwaddch(board_win, bottom, left, ACS_LLCORNER);
	for (int x = left+1; x < right; x++) waddch(board_win, ACS_HLINE);
	waddch(board_win, ACS_LRCORNER);

	if (val >= 256)   wattroff(board_win, A_BOLD);
	if (val >= 32768) wattroff(board_win, A_UNDERLINE);
}



void draw_board(WINDOW *board_win, board_t board, int is_gameover)
{
	for (int y = 0; y < 4; y++) {
		for (int x = 0; x < 4; x++) {
			int xc = TILE_WIDTH  * x + 1;
			int yc = TILE_HEIGHT * y + 1;
			draw_tile(board_win, yc, xc, board[y][x]);
		}
	}
	if (is_gameover) {
		wattron(board_win, A_BOLD | COLOR_PAIR(1));	
		mvwprintw(board_win, TILE_HEIGHT*2, (TILE_WIDTH*4 - 8) / 2,  "GAME OVER");
		wattroff(board_win, A_BOLD);
	}
}

void draw_score(WINDOW *score_win, int score, int points, int max_score)
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
	mvwprintw(score_win, 15, 2, "estart");
	mvwprintw(score_win, 16, 2, "uit");

	wattron(score_win, COLOR_PAIR(3));
	mvwaddch(score_win, 15, 1, 'R');
	
	wattron(score_win, COLOR_PAIR(7));
	mvwaddch(score_win, 16, 1, 'Q');
}

typedef struct { // sliding tile
	int x, y; // coords
	int tick; // move every tick
	int val;  // tile's number
} tile_t;

/* what tile to draw first?
   if sliding left, draw tiles from left to right,
   same for other directions */
static int sort_left(const void *l, const void *r)
{
	return ((tile_t *)l)->x - ((tile_t *)r)->x;
}
static int sort_right(const void *l, const void *r)
{
	return ((tile_t *)r)->x - ((tile_t *)l)->x;
}
static int sort_up(const void *l, const void *r)
{
	return ((tile_t *)l)->y - ((tile_t *)r)->y;
}
static int sort_down(const void *l, const void *r)
{
	return ((tile_t *)r)->y - ((tile_t *)l)->y;
}
				
void draw_slide(WINDOW *board_win, board_t board, board_t moves, int dir)
{
	tile_t tiles[16]; // sliding tiles
	int tiles_n = 0;

	for (int y = 0; y < 4; y++) {
		for (int x = 0; x < 4; x++) {
			if (moves[y][x]) {
				tiles[tiles_n] = (tile_t){.x = x*TILE_WIDTH + 1, .y = y*TILE_HEIGHT + 1,
									.val = board[y][x],
									.tick = 6 / moves[y][x]};
				tiles_n++;
				// remove sliding tiles from the board
				board[y][x] = 0;
			}
		}
	}

	int (*sort)(const void*, const void*);
	int mx = 0, my = 0; //coord modifiers
	switch (dir) {
		case 'l': sort = sort_left;  mx = -2; break;
		case 'r': sort = sort_right; mx =  2; break;
		case 'u': sort = sort_up;    my = -1; break;
		case 'd': sort = sort_down;  my =  1; break;
		default : exit(1); break;
	}
	qsort(tiles, tiles_n, sizeof(tile_t), sort);
	
	nanosleep(&tick_time, NULL);
	/* sliding continues for 30 ticks,
	   a tile can move every 1, 2 or 3 ticks */
	for (int tick = 1; tick <= 30; tick++) {
		for (int t = 0; t < tiles_n; t++) {
			if (tick % tiles[t].tick == 0) { //time to move the tile
				tiles[t].x += mx;
				tiles[t].y += my;
			}
		}
		draw_board(board_win, board, 0); // draw static tiles
		for (int t = 0; t < tiles_n; t++) { // draw moving tiles
			draw_tile(board_win, tiles[t].y, tiles[t].x, tiles[t].val);
		}
		wrefresh(board_win);
		nanosleep(&tick_time, NULL);
	}
	nanosleep(&end_move_time, NULL);
}

