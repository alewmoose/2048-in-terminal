#define _GNU_SOURCE
#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include "draw.h"


typedef struct tile {
	int x, y;
	int tick;
	int val;
} Tile;


static const char *tile_str[] = { "        ",
	"   2    ", "   4    ", "   8    ", "   16   ",
	"   32   ", "   64   ", "  128   ", "  256   ",
	"  512   ", "  1024  ", "  2048  ", "  4096  ",
	"  8192  ", " 16384  ", " 32768  ", " 65536  ",
	" 131072 "
};
static const char *empty_tile_str = "          ";

static const NCURSES_ATTR_T  tile_attr[] = {COLOR_PAIR(1),       // emtpy tile
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

static WINDOW *board_win;
static WINDOW *stats_win;


int init_win()
{
	if (board_win) {
		delwin(board_win);
		board_win = NULL;
	}
	if (stats_win) {
		delwin(stats_win);
		stats_win = NULL;
	}

	clear();
	refresh();

	int scr_width, scr_height;
	getmaxyx(stdscr, scr_height, scr_width);

	int board_win_width  = TILE_WIDTH  * BOARD_SIZE + 2;
	int stats_win_width  = 13;

	int board_win_height = TILE_HEIGHT * BOARD_SIZE + 2;
	int stats_win_height = board_win_height - 2;

	int board_win_top    = (scr_height - board_win_height) / 2;
	int stats_win_top    = board_win_top + 1;

	int board_win_left;
	if (board_win_width + stats_win_width < scr_width)
		board_win_left = (scr_width - board_win_width - stats_win_width) / 2;
	else
		board_win_left = 0;
	int stats_win_left   = board_win_left + board_win_width + 1;


	if (board_win_height > scr_height || board_win_width > scr_width) {
		return WIN_TOO_SMALL;
	}

	board_win = newwin(board_win_height, board_win_width,
	                   board_win_top, board_win_left);
	wborder(board_win, ACS_VLINE, ACS_VLINE, ACS_HLINE, ACS_HLINE,
	        ACS_ULCORNER, ACS_URCORNER, ACS_LLCORNER, ACS_LRCORNER);

	stats_win = newwin(stats_win_height, stats_win_width,
	                   stats_win_top, stats_win_left);

	if (!board_win || !stats_win) {
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

static void draw_stats(const Stats *stats);
static void draw_board(const Board *board);
static void draw_tile(int top, int left, int val);

void draw(const Board *board, const Stats *stats)
{
	if (board) {
		draw_board(board);
		if (stats && stats->game_over) {
			wattron(board_win, A_BOLD | COLOR_PAIR(1));
			mvwprintw(board_win, TILE_HEIGHT*2, (TILE_WIDTH*BOARD_SIZE - 8) / 2,
		          	  "GAME OVER");
			wattroff(board_win, A_BOLD);
		}
		wrefresh(board_win);
	}
	if (stats) {
		draw_stats(stats);
		wrefresh(stats_win);
	}
}

static void draw_board(const Board *board)
{
	for (int y = 0; y < BOARD_SIZE; y++) {
		for (int x = 0; x < BOARD_SIZE; x++) {
			if (board->tiles[y][x] == -1)
				continue;
			int xc = TILE_WIDTH  * x + 1;
			int yc = TILE_HEIGHT * y + 1;
			draw_tile(yc, xc, board->tiles[y][x]);
		}
	}
}

static void draw_stats(const Stats *stats)
{
	wattron(stats_win, COLOR_PAIR(2));
	mvwprintw(stats_win, 1, 1, "Score");
	mvwprintw(stats_win, 4, 1, "Best Score");

	if (stats->points > 0) {
		wattron(stats_win, COLOR_PAIR(3));
		mvwprintw(stats_win, 1, 7, "%+6d", stats->points);
	} else {
		mvwprintw(stats_win, 1, 7, "       ");
	}

	if (!stats->auto_save) {
		wattron(stats_win, COLOR_PAIR(1));
		mvwprintw(stats_win, 8, 1, "Autosave is");
		wattron(stats_win, COLOR_PAIR(7));
		mvwprintw(stats_win, 9, 9, "OFF");
	}

	wattron(stats_win, COLOR_PAIR(1));
	mvwprintw(stats_win, 2, 1, "%8d", stats->score);
	mvwprintw(stats_win, 5, 1, "%8d", stats->max_score);
	mvwprintw(stats_win, 15, 2, "estart");
	mvwprintw(stats_win, 16, 2, "uit");

	wattron(stats_win, COLOR_PAIR(3));
	mvwaddch(stats_win, 15, 1, 'R');

	wattron(stats_win, COLOR_PAIR(7));
	mvwaddch(stats_win, 16, 1, 'Q');
}


static void draw_tile(int top, int left, int val)
{
	int right  = left + TILE_WIDTH  - 1;
	int bottom = top  + TILE_HEIGHT - 1;
	int center = (top + bottom) / 2;

	/* draw empty tile */
	if (val == 0) {
		for (int y = top; y <= bottom; y++)
			mvwprintw(board_win, y, left, empty_tile_str);
		return;
	}

	wattrset(board_win, tile_attr[val]);

	/* erase tile except it's board */
	for (int y = top+1; y < bottom; y++)
		mvwprintw(board_win, y, left+1, tile_str[0]);

	/* draw corners */
	mvwaddch(board_win, top,    left,  ACS_ULCORNER);
	mvwaddch(board_win, top,    right, ACS_URCORNER);
	mvwaddch(board_win, bottom, left,  ACS_LLCORNER);
	mvwaddch(board_win, bottom, right, ACS_LRCORNER);

	/* draw lines */
	mvwhline(board_win, top,    left+1, ACS_HLINE, TILE_WIDTH-2);
	mvwhline(board_win, bottom, left+1, ACS_HLINE, TILE_WIDTH-2);
	mvwvline(board_win, top+1, left,  ACS_VLINE, TILE_HEIGHT-2);
	mvwvline(board_win, top+1, right, ACS_VLINE, TILE_HEIGHT-2);

	/* draw number */
	mvwprintw(board_win, center, left+1, tile_str[val]);
}


static int sort_left(const void *l, const void *r);
static int sort_right(const void *l, const void *r);
static int sort_up(const void *l, const void *r);
static int sort_down(const void *l, const void *r);

void draw_slide(Board *board, const Board *moves, Dir dir)
{
	Tile tiles[BOARD_TILES]; /* sliding tiles */
	int tiles_n = 0;

	for (int y = 0; y < BOARD_SIZE; y++) {
		for (int x = 0; x < BOARD_SIZE; x++) {
			if (moves->tiles[y][x] == 0)
				continue;
			Tile tile;
			/* convert board position to window coords */
			tile.x = x * TILE_WIDTH + 1;
			tile.y = y * TILE_HEIGHT + 1;
			tile.val = board->tiles[y][x];
			tile.tick = 6 / moves->tiles[y][x];
			/* remove sliding tiles from the board */
			board->tiles[y][x] = 0;
			tiles[tiles_n++] = tile;
		}
	}

	int (*sort)(const void*, const void*);
	int mx = 0, my = 0; /* coord modifiers */
	switch (dir) {
	case LEFT:  sort = sort_left;  mx = -2; break;
	case RIGHT: sort = sort_right; mx =  2; break;
	case UP:    sort = sort_up;    my = -1; break;
	case DOWN:  sort = sort_down;  my =  1; break;
	default :   exit(1); break;
	}
	/* sort sliding tiles according to direction */
	qsort(tiles, tiles_n, sizeof(Tile), sort);

	nanosleep(&tick_time, NULL);
	/* sliding continues for 30 ticks,
	   a tile can move every 1, 2 or 3 ticks */
	for (int tick = 1; tick <= 30; tick++) {
		for (int t = 0; t < tiles_n; t++) {
			if (tick % tiles[t].tick != 0)
				continue;
			/* move tile */
			draw_tile(tiles[t].y, tiles[t].x, 0);
			tiles[t].x += mx;
			tiles[t].y += my;
			draw_tile(tiles[t].y, tiles[t].x, tiles[t].val);
		}
		wrefresh(board_win);
		nanosleep(&tick_time, NULL);
	}
	nanosleep(&end_move_time, NULL);
}

	

/* what tile to draw first?
   if sliding left, draw sliding tiles from left to right,
   same for other directions */
static int sort_left(const void *l, const void *r)
{
	return ((Tile *)l)->x - ((Tile *)r)->x;
}
static int sort_right(const void *l, const void *r)
{
	return ((Tile *)r)->x - ((Tile *)l)->x;
}
static int sort_up(const void *l, const void *r)
{
	return ((Tile *)l)->y - ((Tile *)r)->y;
}
static int sort_down(const void *l, const void *r)
{
	return ((Tile *)r)->y - ((Tile *)l)->y;
}
