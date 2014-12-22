#define _GNU_SOURCE
#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include "draw.h"


static const char *tile_str[] = { "        ",
	"   2    ", "   4    ", "   8    ", "   16   ",
	"   32   ", "   64   ", "  128   ", "  256   ",
	"  512   ", "  1024  ", "  2048  ", "  4096  ",
	"  8192  ", " 16384  ", " 32768  ", " 65536  ",
	" 131072 "
};

static const NCURSES_ATTR_T  tile_attr[] = { COLOR_PAIR(1),       // emtpy tile
	COLOR_PAIR(1), COLOR_PAIR(2), COLOR_PAIR(3), COLOR_PAIR(4),// 2 4 8 16
	COLOR_PAIR(5), COLOR_PAIR(6), COLOR_PAIR(7),               // 32 64 128

	COLOR_PAIR(1) | A_BOLD, COLOR_PAIR(2) | A_BOLD,           // 256  512
	COLOR_PAIR(3) | A_BOLD, COLOR_PAIR(4) | A_BOLD,           // 1024 2048
	COLOR_PAIR(5) | A_BOLD, COLOR_PAIR(6) | A_BOLD,           // 4096 8192
	COLOR_PAIR(7) | A_BOLD,                                   // 16384

	COLOR_PAIR(1) | A_BOLD,                     // 32768
	COLOR_PAIR(2) | A_BOLD,                     // 65536
	COLOR_PAIR(3) | A_BOLD,                     // 131072
};

static const struct timespec tick_time     = {.tv_sec = 0, .tv_nsec = 5000000};
static const struct timespec end_move_time = {.tv_sec = 0, .tv_nsec = 6000000};
extern bool autosave;


bool init_win(WINDOW **board_win, WINDOW **score_win)
{
	if (*board_win) delwin(*board_win);
	if (*score_win) delwin(*score_win);
	*board_win = NULL;
	*score_win = NULL;

	clear();
	refresh();

	int scr_width, scr_height;
	getmaxyx(stdscr, scr_height, scr_width);

	int board_win_width  = TILE_WIDTH  * BOARD_SIZE + 2;
	int score_win_width  = 13;

	int board_win_height = TILE_HEIGHT * BOARD_SIZE + 2;
	int score_win_height = board_win_height - 2;

	int board_win_top    = (scr_height - board_win_height) / 2;
	int score_win_top    = board_win_top + 1;

	int board_win_left;
	if (board_win_width + score_win_width < scr_width) {
		board_win_left = (scr_width - board_win_width - score_win_width) / 2;
	} else {
		board_win_left = 0;
	}
	int score_win_left   = board_win_left + board_win_width + 1;


	if (board_win_height > scr_height || board_win_width > scr_width) {
		return WIN_TOO_SMALL;
	}

	*board_win = newwin(board_win_height, board_win_width,
	                    board_win_top, board_win_left);
	wborder(*board_win, ACS_VLINE, ACS_VLINE, ACS_HLINE, ACS_HLINE,
	        ACS_ULCORNER, ACS_URCORNER, ACS_LLCORNER, ACS_LRCORNER);


	*score_win = newwin(score_win_height, score_win_width,
	                    score_win_top, score_win_left);

	if (!board_win || !score_win) { //nothing i can do about it
		endwin();
		exit(1);
	}
	return WIN_OK;
}

void setup_screen(void)
{
	initscr();
	start_color();
	noecho();
	cbreak();
	curs_set(0);
	set_escdelay(0);
	keypad(stdscr, true);

	init_pair(1, COLOR_WHITE,   COLOR_BLACK);
	init_pair(2, COLOR_YELLOW,  COLOR_BLACK);
	init_pair(3, COLOR_GREEN,   COLOR_BLACK);
	init_pair(4, COLOR_BLUE,    COLOR_BLACK);
	init_pair(5, COLOR_MAGENTA, COLOR_BLACK);
	init_pair(6, COLOR_CYAN,    COLOR_BLACK);
	init_pair(7, COLOR_RED,     COLOR_BLACK);
}

void print_too_small(void)
{
	static const char *msg = "TERMINAL TOO SMALL";
	static int mlen = 0;
	if (!mlen)
		mlen = strlen(msg);

	int width, height;
	getmaxyx(stdscr, height, width);
	int x = (width - mlen) / 2;
	x = x >= 0 ? x : 0;
	int y = height / 2;
	mvprintw(y, x, msg);
	refresh();
}

static void draw_tile(WINDOW *board_win, int top, int left, int val)
{
	int right  = left + TILE_WIDTH  - 1;
	int bottom = top  + TILE_HEIGHT - 1;

	// draw empty tile
	if (val == 0) {
		for (int y = top; y <= bottom; y++) {
			// 10 spaces
			mvwprintw(board_win, y, left, "          ");
		}
		return;
	}

	wattrset(board_win, tile_attr[val]);

	// draw top line
	mvwaddch(board_win, top, left, ACS_ULCORNER);
	for (int x = left+1; x < right; x++)
		waddch(board_win, ACS_HLINE);
	waddch(board_win, ACS_URCORNER);

	for (int y = top+1; y < bottom; y++) {
		mvwaddch(board_win, y, left, ACS_VLINE);
		if (y == (top + bottom) / 2) {  //central line with number
			wprintw(board_win, tile_str[val]);
		} else {
			wprintw(board_win, tile_str[0]); // 8 spaces
		}
		waddch(board_win, ACS_VLINE);
	}

	// draw bottom line
	mvwaddch(board_win, bottom, left, ACS_LLCORNER);
	for (int x = left+1; x < right; x++) waddch(board_win, ACS_HLINE);
	waddch(board_win, ACS_LRCORNER);
}



static void draw_board(WINDOW *board_win, board_t board, bool is_gameover)
{
	for (int y = 0; y < BOARD_SIZE; y++) {
		for (int x = 0; x < BOARD_SIZE; x++) {
			int xc = TILE_WIDTH  * x + 1;
			int yc = TILE_HEIGHT * y + 1;
			draw_tile(board_win, yc, xc, board[y][x]);
		}
	}
	if (is_gameover) {
		wattron(board_win, A_BOLD | COLOR_PAIR(1));
		mvwprintw(board_win, TILE_HEIGHT*2, (TILE_WIDTH*BOARD_SIZE - 8) / 2,
		          "GAME OVER");
		wattroff(board_win, A_BOLD);
	}
}

static void draw_score(WINDOW *score_win, int score, int points, int max_score)
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

	if (!autosave) {
		wattron(score_win, COLOR_PAIR(1));
		mvwprintw(score_win, 8, 1, "Autosave is");
		wattron(score_win, COLOR_PAIR(7));
		mvwprintw(score_win, 9, 9, "OFF");
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

inline void refresh_board(WINDOW *board_win, board_t board, bool is_gameover)
{
	draw_board(board_win, board, is_gameover);
	wrefresh(board_win);
}

inline void refresh_score(WINDOW *score_win, int score,
                          int points, int max_score)
{
	draw_score(score_win, score, points, max_score);
	wrefresh(score_win);
}

typedef struct { // sliding tile
	int x, y; // coords
	int tick; // move every tick
	int val;  // tile's number
} tile_t;

/* what tile to draw first?
   if sliding left, draw sliding tiles from left to right,
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

void draw_slide(WINDOW *board_win, board_t board, board_t moves, dir_t dir)
{
	tile_t tiles[BOARD_TILES]; // sliding tiles
	int tiles_n = 0;

	for (int y = 0; y < BOARD_SIZE; y++) {
		for (int x = 0; x < BOARD_SIZE; x++) {
			if (moves[y][x]) {
				tile_t tile;
				// convert board position to window coords
				tile.x = x * TILE_WIDTH + 1;
				tile.y = y * TILE_HEIGHT + 1;
				tile.val = board[y][x];
				tile.tick = 6 / moves[y][x];
				// remove sliding tiles from the board
				board[y][x] = 0;
				tiles[tiles_n++] = tile;
			}
		}
	}

	int (*sort)(const void*, const void*);
	int mx = 0, my = 0; //coord modifiers
	switch (dir) {
		case LEFT:  sort = sort_left;  mx = -2; break;
		case RIGHT: sort = sort_right; mx =  2; break;
		case UP:    sort = sort_up;    my = -1; break;
		case DOWN:  sort = sort_down;  my =  1; break;
		default :   exit(1); break;
	}
	//sort sliding tiles according to direction
	qsort(tiles, tiles_n, sizeof(tile_t), sort);

	nanosleep(&tick_time, NULL);
	/* sliding continues for 30 ticks,
	   a tile can move every 1, 2 or 3 ticks */
	for (int tick = 1; tick <= 30; tick++) {
		for (int t = 0; t < tiles_n; t++) {
			//time to move the tile
			if (tick % tiles[t].tick == 0) {
				tiles[t].x += mx;
				tiles[t].y += my;
			}
		}
		draw_board(board_win, board, 0); // draw static tiles
		for (int t = 0; t < tiles_n; t++) { // draw moving tiles
			draw_tile(board_win, tiles[t].y,
			          tiles[t].x, tiles[t].val);
		}
		wrefresh(board_win);
		nanosleep(&tick_time, NULL);
	}
	nanosleep(&end_move_time, NULL);
}

