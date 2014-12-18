#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "board.h"

static const int tile_num[] = { 0,
	2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048,
	4096, 8192, 16384, 32768, 65536, 131072
};
								
inline void board_copy(board_t dest, board_t source)
{
	memcpy(dest, source, 16*sizeof(int));
}

void board_add_tile(board_t board, int only2)
{
	int emptyx[16], emptyy[16], empty_n = 0;
	int x, y, val;
// 12.5% chance of getting '4'
	if (only2) {
		val = 1;
	} else {
		val = (rand() % 8 == 1) ? 2 : 1;
	}

	for (int y = 0; y < 4; y++) {
		for (int x = 0; x < 4; x++) {
			if (board[y][x] == 0) {
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
		board[y][x] = val;
	}
}

void board_start(board_t board)
{
	memset(board, 0, 16*sizeof(int));
	board_add_tile(board, 1); // add only 2's on start
	board_add_tile(board, 1);
}

static void rotate_l(board_t board)
{
	board_t tmp;
	board_copy(tmp, board);
	for (int y = 0; y < 4; y++) {
		for (int x = 0; x < 4; x++)
			board[3-x][y] = tmp[y][x];
	}
}

static void rotate_r(board_t board)
{
	board_t tmp;
	board_copy(tmp, board);
	for (int y = 0; y < 4; y++) {
		for (int x = 0; x < 4; x++)
			board[x][3-y] = tmp[y][x];
	}
}

static void rotate_2(board_t board)
{
	board_t tmp;
	board_copy(tmp, board);
	for (int y = 0; y < 4; y++) {
		for (int x = 0; x < 4; x++)
			board[y][3-x] = tmp[y][x];
	}
}

static int slide_left(board_t board, board_t moves)
{
	/* returns points or -1 if didn't slide */
	memset(moves, 0, 16*sizeof(int));
	int points = 0, slided = 0;

	for (int y = 0; y < 4; y++) {
		int *row = board[y];
		for (int x = 0; x < 3; x++) {
			if (row[x] == 0) { // found an empty spot, move next square here
				int next;
				for (next = x+1; next < 4 && row[next] == 0; next++);
				if (next < 4) {
					if (!slided) slided = 1;
					row[x] = row[next];
					row[next] = 0;
					moves[y][next] = next - x;
				}
			}
			if (row[x] != 0) { // search for square with same num
				int next;
				for (next = x+1; next < 4 && row[next] == 0; next++);
				if (next < 4 && row[x] == row[next]) {
					if (!slided) slided = 1;
					row[x]++;
					points += tile_num[row[x]];
					row[next] = 0;
					moves[y][next] = next - x;
				}
			}
		}
	}

	return slided ? points : -1;
}

int board_slide(board_t board, board_t new_board, board_t moves,  int dir)
{
	/* returns points or -1 if didn't slide, stores moves for animation */
	board_copy(new_board, board);

	// rotate field
	if (dir == 'r')      rotate_2(new_board);
	else if (dir == 'u') rotate_l(new_board);
	else if (dir == 'd') rotate_r(new_board);

	int points = slide_left(new_board, moves);
	if (points == -1) return -1;
	
	// rotate back
	if (dir == 'r') {
		rotate_2(new_board);
		rotate_2(moves);
	} else if (dir == 'u') {
		rotate_r(new_board);
		rotate_r(moves);
	} else if (dir == 'd') {
		rotate_l(new_board);
		rotate_l(moves);
	}

	return points;
}


int board_can_slide(board_t board)
{
	board_t b1, b2; // dummies
	if (board_slide(board, b1, b2, 'l') >= 0 ||
		board_slide(board, b1, b2, 'r') >= 0 ||
		board_slide(board, b1, b2, 'u') >= 0 ||
		board_slide(board, b1, b2, 'd') >= 0) {
		return 1;
	}
	return 0;
}




static char save_file[256];

static int get_home(void)
{
	char *home = getenv("HOME");
	if (!home || strlen(home) > 249) return 0;

	strcpy(save_file, home);
	strcat(save_file, "/.2048"); 
	return 1;
}

int save_game(board_t board, int score, int max_score)
{
	FILE *fout;
	if (!(fout = fopen(save_file, "w"))) return 0;

	fprintf(fout, "%d\n%d\n", score, max_score);
	for (int y = 0; y < 4; y++) {
		for (int x = 0; x < 4; x++) {
			fprintf(fout, "%d ", board[y][x]);
		}
		fputs("\n", fout);
	}
	fclose(fout);
	return 1;
}

int load_game(board_t board, int *score, int *max_score)
{
	if (!get_home()) return 0;

	FILE *fin;
	if (!(fin = fopen(save_file, "r"))) return 0;
	 
	if (fscanf(fin, "%d%d", score, max_score) != 2)
		return 0;
	int max_possible = 3932156;
	if (*score > *max_score || *score < 0 || *max_score < 0 ||
		*max_score > max_possible) 
		return 0;

	for (int y = 0; y < 4; y++) {
		for (int x = 0; x < 4; x++) {
			int num;
			if (fscanf(fin, "%d", &num) == 0)
				return 0;
			if (num < 0 || num > 17) return 0;
			board[y][x] = num;
		}
	}

	fclose(fin);
	return 1;
} 
