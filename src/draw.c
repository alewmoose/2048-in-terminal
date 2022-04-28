#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include "draw.h"

/* sliding tile */
typedef struct tile {
	int x, y;
	int mx, my; /* coord modifiers */
	int val;    /* tile's value, power of two */
} Tile;


static const char *tile_str[] = { "        ",
	"   2    ", "   4    ", "   8    ", "   16   ",
	"   32   ", "   64   ", "  128   ", "  256   ",
	"  512   ", "  1024  ", "  2048  ", "  4096  ",
	"  8192  ", " 16384  ", " 32768  ", " 65536  ",
	" 131072 "
};
static const char *empty_tile_str = "          ";

static const NCURSES_ATTR_T  tile_attr[] = {
	COLOR_PAIR(1), COLOR_PAIR(1), /* empty 2 */
	COLOR_PAIR(2), COLOR_PAIR(3), COLOR_PAIR(4), /*  4  8  16 */
	COLOR_PAIR(5), COLOR_PAIR(6), COLOR_PAIR(7), /* 32 64 128 */
	COLOR_PAIR(1) | A_BOLD, COLOR_PAIR(2) | A_BOLD, /* 256  512 */
	COLOR_PAIR(3) | A_BOLD, COLOR_PAIR(4) | A_BOLD, /* 1024 2048 */
	COLOR_PAIR(5) | A_BOLD, COLOR_PAIR(6) | A_BOLD, /* 4096 8192 */
	COLOR_PAIR(7) | A_BOLD, /* 16384 */
	COLOR_PAIR(1) | A_BOLD, /* 32768 */
	COLOR_PAIR(2) | A_BOLD, /* 65536 */
	COLOR_PAIR(3) | A_BOLD, /* 131072 */
};

static const struct timespec tick_time     = {.tv_sec = 0, .tv_nsec = 30000000};
static const struct timespec end_move_time = {.tv_sec = 0, .tv_nsec = 6000000};

static WINDOW *board_win;
static WINDOW *stats_win;


int init_win()
{

	static const int bwidth  = TILE_WIDTH  * BOARD_SIZE + 2;
	static const int bheight = TILE_HEIGHT * BOARD_SIZE + 2;
	static const int swidth  = 13;
	static const int sheight = bheight - 2;

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

	int btop = (scr_height - bheight) / 2;
	int stop = btop + 1;

	int bleft;
	if (bwidth + swidth < scr_width)
		bleft = (scr_width - bwidth - swidth) / 2;
	else
		bleft = 0;
	int sleft = bleft + bwidth + 1;


	if (bheight > scr_height || bwidth > scr_width)
		return WIN_TOO_SMALL;

	board_win = newwin(bheight, bwidth, btop, bleft);
	stats_win = newwin(sheight, swidth, stop, sleft);
	if (!board_win || !stats_win) {
		endwin();
		exit(1);
	}
	wattrset(board_win, COLOR_PAIR(1));
	wborder(board_win, ACS_VLINE, ACS_VLINE, ACS_HLINE, ACS_HLINE,
	        ACS_ULCORNER, ACS_URCORNER, ACS_LLCORNER, ACS_LRCORNER);

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
	mvprintw(y, x, "%s", msg);
	refresh();
}

static void draw_stats(const Stats *stats);
static void draw_board(const Board *board);
/* 'top' and 'left' are window coords of upper-left tile's corner */
static void draw_tile(int top, int left, int val);

void draw(const Board *board, const Stats *stats)
{
	if (board) {
		draw_board(board);
		if (stats && stats->game_over) {
			wattron(board_win, A_BOLD | COLOR_PAIR(1));
			mvwprintw(board_win, TILE_HEIGHT*2,
				  (TILE_WIDTH*BOARD_SIZE - 8) / 2,
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
			/* convert board position to window coords */
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
	mvwprintw(stats_win, 14, 2, "nimations");
	mvwprintw(stats_win, 15, 2, "estart");
	mvwprintw(stats_win, 16, 2, "uit");

	wattron(stats_win, COLOR_PAIR(5));
	mvwaddch(stats_win, 14, 1, 'A');

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
			mvwprintw(board_win, y, left, "%s", empty_tile_str);
		return;
	}

	wattrset(board_win, tile_attr[val]);

	/* erase tile except it's border */
	for (int y = top+1; y < bottom; y++)
		mvwprintw(board_win, y, left+1, "%s", tile_str[0]);

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
	mvwprintw(board_win, center, left+1, "%s", tile_str[val]);
}

/* Passed to qsort */
static int sort_left(const void *l, const void *r);
static int sort_right(const void *l, const void *r);
static int sort_up(const void *l, const void *r);
static int sort_down(const void *l, const void *r);

void draw_slide(const Board *board, const Board *moves, Dir dir)
{
	Tile tiles[BOARD_TILES]; /* sliding tiles */
	int tiles_n = 0;

	for (int y = 0; y < BOARD_SIZE; y++) {
		for (int x = 0; x < BOARD_SIZE; x++) {
			if (moves->tiles[y][x] == 0)
				continue;
			Tile tile;
			int step = moves->tiles[y][x];
			/* convert board position to window coords */
			tile.x = x * TILE_WIDTH + 1;
			tile.y = y * TILE_HEIGHT + 1;
			tile.val = board->tiles[y][x];

			switch (dir) {
			case UP:    tile.mx = 0; tile.my = -1 * step; break;
			case DOWN:  tile.mx = 0; tile.my =  1 * step; break;
			case LEFT:  tile.mx = -2 * step; tile.my = 0; break;
			case RIGHT: tile.mx =  2 * step; tile.my = 0; break;
			}
			tiles[tiles_n++] = tile;
		}
	}

	int (*sort)(const void*, const void*);
	switch (dir) {
	case LEFT:  sort = sort_left;  break;
	case RIGHT: sort = sort_right; break;
	case UP:    sort = sort_up;    break;
	case DOWN:  sort = sort_down;  break;
	default :   exit(1); break;
	}
	/* sort sliding tiles according to direction */
	qsort(tiles, tiles_n, sizeof(Tile), sort);

	nanosleep(&tick_time, NULL);
	for (int tick = 1; tick <= 5; tick++) {
		for (int t = 0; t < tiles_n; t++) {
			/* erase */
			draw_tile(tiles[t].y, tiles[t].x, 0);
			/* move */
			tiles[t].x += tiles[t].mx;
			tiles[t].y += tiles[t].my;
			/* redraw */
			draw_tile(tiles[t].y, tiles[t].x, tiles[t].val);
		}
		wrefresh(board_win);
		nanosleep(&tick_time, NULL);
	}
	nanosleep(&end_move_time, NULL);
}


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
