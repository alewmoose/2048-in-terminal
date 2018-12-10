#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "board.h"

static const int tile_num[] = {0,
	2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048,
	4096, 8192, 16384, 32768, 65536, 131072
};

void board_start(Board *board)
{
	memset(board, 0, sizeof(Board));
	/* add only 2's on start */
	board_add_tile(board, true);
	board_add_tile(board, true);
}

void board_add_tile(Board *board, bool only2)
{
	Coord empty[BOARD_TILES];
	int empty_n = 0;
	int val;

	if (only2) {
		val = 1;
	} else {
		/* 10% chance of getting '4' */
		val = (rand() % 10 == 1) ? 2 : 1;
	}

	for (int y = 0; y < BOARD_SIZE; y++) {
		for (int x = 0; x < BOARD_SIZE; x++) {
			if (board->tiles[y][x] == 0) {
				empty[empty_n].x = x;
				empty[empty_n].y = y;
				empty_n++;
			}
		}
	}

	if (empty_n > 0) {
		int r = rand() % empty_n;
		int x = empty[r].x;
		int y = empty[r].y;
		board->tiles[y][x] = val;
	}
}

static void rotate_l(Board *board);
static void rotate_r(Board *board);
static void rotate_2(Board *board);
static int  slide_left(Board *board, Board *moves);


int board_slide(const Board *board, Board *new_board, Board *moves,  Dir dir)
{
	*new_board = *board;

	/* rotate board */
	switch (dir) {
	case RIGHT: rotate_2(new_board); break;
	case UP:    rotate_l(new_board); break;
	case DOWN:  rotate_r(new_board); break;
	case LEFT:  break;
	}

	int points = slide_left(new_board, moves);
	if (points == NO_SLIDE)
		return NO_SLIDE;

	/* rotate back */
	switch (dir) {
	case RIGHT:
		rotate_2(new_board);
		rotate_2(moves);
		break;
	case UP:
		rotate_r(new_board);
		rotate_r(moves);
		break;
	case DOWN:
		rotate_l(new_board);
		rotate_l(moves);
		break;
	case LEFT: break;
	}

	return points;
}
bool board_can_slide(const Board *board)
{
	Board b1, b2; /* dummies */
	if (board_slide(board, &b1, &b2, LEFT)  == NO_SLIDE &&
	    board_slide(board, &b1, &b2, RIGHT) == NO_SLIDE &&
	    board_slide(board, &b1, &b2, UP)    == NO_SLIDE &&
	    board_slide(board, &b1, &b2, DOWN)  == NO_SLIDE) {
		return false;
	}
	return true;
}

static void rotate_l(Board *board)
{
	Board tmp = *board;
	for (int y = 0; y < BOARD_SIZE; y++) {
		for (int x = 0; x < BOARD_SIZE; x++)
			board->tiles[BOARD_SIZE-1-x][y] = tmp.tiles[y][x];
	}
}

static void rotate_r(Board *board)
{
	Board tmp = *board;
	for (int y = 0; y < BOARD_SIZE; y++) {
		for (int x = 0; x < BOARD_SIZE; x++)
			board->tiles[x][BOARD_SIZE-1-y] = tmp.tiles[y][x];
	}
}

static void rotate_2(Board *board)
{
	Board tmp = *board;
	for (int y = 0; y < BOARD_SIZE; y++) {
		for (int x = 0; x < BOARD_SIZE; x++)
			board->tiles[y][BOARD_SIZE-1-x] = tmp.tiles[y][x];
	}
}


static int next_same(int row[BOARD_SIZE], int val, int start);
static int next_any (int row[BOARD_SIZE], int start);

/* returns points or NO_SLIDE if didn't slide */
static int slide_left(Board *board, Board *moves)
{
	memset(moves, 0, sizeof(Board));
	int  points = 0;
	bool slided = false;

	for (int y = 0; y < BOARD_SIZE; y++) {
		int *row = board->tiles[y];
		for (int x = 0; x < BOARD_SIZE-1; x++) {
			if (row[x] == 0) {
				int next = next_any(row, x + 1);
				if (next == -1)
					goto NEXT_ROW;

				/* found an empty spot, move next tile here */
				slided = true;
				row[x] = row[next];
				row[next] = 0;
				moves->tiles[y][next] = next - x;
			}
			if (row[x] != 0) {
				int next = next_same(row, row[x], x + 1);
				if (next == -1)
					continue;

				/* found same tile, merge */
				slided = true;
				row[x]++;
				row[next] = 0;
				points += tile_num[row[x]];
				moves->tiles[y][next] = next - x;
			}
		}
	NEXT_ROW: ;
	}

	return slided ? points : NO_SLIDE;
}

static int next_any(int row[BOARD_SIZE], int start)
{
	for (int i = start; i < BOARD_SIZE; i++)
		if (row[i] > 0)
			return i;
	return -1;
}

static int next_same(int row[BOARD_SIZE], int val, int start)
{
	for (int i = start; i < BOARD_SIZE; i++) {
		if (row[i] == val)
			return i;
		else if (row[i] != 0)
			return -1;
	}
	return -1;
}
