#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "board.h"

static const int tile_num[] = { 0,
	2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048,
	4096, 8192, 16384, 32768, 65536, 131072
};

inline void board_copy(Board *dest, const Board *source)
{
	memcpy(dest, source, BOARD_TILES*sizeof(int));
}

void board_add_tile(Board *board, bool only2)
{
	int emptyx[BOARD_TILES], emptyy[BOARD_TILES], empty_n = 0;
	int x, y, val;
// 12.5% chance of getting '4'
	if (only2) {
		val = 1;
	} else {
		val = (rand() % 8 == 1) ? 2 : 1;
	}

	for (int y = 0; y < BOARD_SIZE; y++) {
		for (int x = 0; x < BOARD_SIZE; x++) {
			if (board->tiles[y][x] == 0) {
				emptyx[empty_n] = x;
				emptyy[empty_n] = y;
				empty_n++;
			}
		}
	}

	if (empty_n > 0) {
		int r = rand() % empty_n;
		x = emptyx[r];
		y = emptyy[r];
		board->tiles[y][x] = val;
	}
}

void board_start(Board *board)
{
	memset(board, 0, BOARD_TILES*sizeof(int));
	board_add_tile(board, true); // add only 2's on start
	board_add_tile(board, true);
}

static void rotate_l(Board *board)
{
	Board tmp;
	board_copy(&tmp, board);
	for (int y = 0; y < BOARD_SIZE; y++) {
		for (int x = 0; x < BOARD_SIZE; x++)
			board->tiles[BOARD_SIZE-1-x][y] = tmp.tiles[y][x];
	}
}

static void rotate_r(Board *board)
{
	Board tmp;
	board_copy(&tmp, board);
	for (int y = 0; y < BOARD_SIZE; y++) {
		for (int x = 0; x < BOARD_SIZE; x++)
			board->tiles[x][BOARD_SIZE-1-y] = tmp.tiles[y][x];
	}
}

static void rotate_2(Board *board)
{
	Board tmp;
	board_copy(&tmp, board);
	for (int y = 0; y < BOARD_SIZE; y++) {
		for (int x = 0; x < BOARD_SIZE; x++)
			board->tiles[y][BOARD_SIZE-1-x] = tmp.tiles[y][x];
	}
}

static int slide_left(Board *board, Board *moves)
{
	/* returns points or NO_SLIDE if didn't slide */
	memset(moves, 0, BOARD_TILES*sizeof(int));
	int points = 0;
	bool slided = false;

	for (int y = 0; y < BOARD_SIZE; y++) {
		int *row = board->tiles[y];
		for (int x = 0; x < BOARD_SIZE-1; x++) {
			if (row[x] == 0) { // found an empty spot, move next tile here
				int next;
				for (next = x+1; next < BOARD_SIZE && row[next] == 0; next++);
				if (next < BOARD_SIZE) {
					if (!slided) slided = 1;
					row[x] = row[next];
					row[next] = 0;
					moves->tiles[y][next] = next - x;
				}
			}
			if (row[x] != 0) { // search for tile with same num
				int next;
				for (next = x+1; next < BOARD_SIZE && row[next] == 0; next++);
				if (next < BOARD_SIZE && row[x] == row[next]) {
					if (!slided) slided = 1;
					row[x]++;
					points += tile_num[row[x]];
					row[next] = 0;
					moves->tiles[y][next] = next - x;
				}
			}
		}
	}

	return slided ? points : NO_SLIDE;
}

int board_slide(const Board *board, Board *new_board, Board *moves,  Dir dir)
{
	/* returns points or NO_SLIDE if didn't slide, stores moves for animation */
	board_copy(new_board, board);

	// rotate field
	if      (dir == RIGHT) rotate_2(new_board);
	else if (dir == UP)    rotate_l(new_board);
	else if (dir == DOWN)  rotate_r(new_board);

	int points = slide_left(new_board, moves);
	if (points == NO_SLIDE)
		goto ext;

	// rotate back
	if (dir == RIGHT) {
		rotate_2(new_board);
		rotate_2(moves);
	} else if (dir == UP) {
		rotate_r(new_board);
		rotate_r(moves);
	} else if (dir == DOWN) {
		rotate_l(new_board);
		rotate_l(moves);
	}
ext:
	return points;
}


bool board_can_slide(const Board *board)
{
	Board b1, b2; // dummies
	if (board_slide(board, &b1, &b2, LEFT)  == NO_SLIDE &&
	    board_slide(board, &b1, &b2, RIGHT) == NO_SLIDE &&
	    board_slide(board, &b1, &b2, UP)    == NO_SLIDE &&
	    board_slide(board, &b1, &b2, DOWN)  == NO_SLIDE) {
		return false;
	}
	return true;
}


