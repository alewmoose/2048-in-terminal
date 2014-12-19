#define _GNU_SOURCE
#include <ncurses.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include "draw.h"
#include "board.h"

int main(void)
{
	setup_screen();
	srand(time(NULL));
	const struct timespec addsquare_time = {.tv_sec = 0, .tv_nsec = 100000000};
	bool terminal_too_small = false;

	board_t board;
	int score = 0, max_score = 0, gameover = false;
	if (!load_game(board, &score, &max_score)) {
		board_start(board);
		score = max_score = 0;
	}

	WINDOW *board_win = NULL, *score_win = NULL;
	if (init_win(&board_win, &score_win) == WIN_TOO_SMALL) {
		terminal_too_small = true;
		print_too_small();
	} else {
		refresh_board(board_win, board, gameover);
		refresh_score(score_win, score, 0, max_score);
	}

	int ch;
	while ((ch = getch()) != 'q' && ch != 'Q') {  // q to quit
		if (terminal_too_small && ch != KEY_RESIZE)
			continue;
		int dir, points = 0;
		board_t new_board = {{0}};
		board_t moves     = {{0}};

		switch(ch) {
			case KEY_UP:    dir = 'u'; break; // moving
			case KEY_DOWN:  dir = 'd'; break;
			case KEY_LEFT:  dir = 'l'; break;
			case KEY_RIGHT: dir = 'r'; break;

			case 'r': case 'R':               // start new game
				score = 0;
				gameover = false;
				board_start(board);
				refresh_board(board_win, board, gameover);
				refresh_score(score_win, score, 0, max_score);
				continue;
				break;

			case KEY_RESIZE:                  // terminal resize
				if (init_win(&board_win, &score_win) == WIN_TOO_SMALL) {
					terminal_too_small = true;
					print_too_small();
					continue;
				} else {
					terminal_too_small = false;
				}
				refresh_board(board_win, board, gameover);
				refresh_score(score_win, score, points, max_score);
				continue;
				break;
			default: continue;
		}

		if (gameover) continue;

		points = board_slide(board, new_board, moves, dir);
		if (points >= 0) {
			refresh_score(score_win, score, points, max_score);
			draw_slide(board_win, board, moves, dir);

			board_copy(board, new_board);
			score += points;
			if (score > max_score)
				max_score = score;
			refresh_board(board_win, board, gameover);
			refresh_score(score_win, score, points, max_score);

			nanosleep(&addsquare_time, NULL);
			board_add_tile(board, false);
			refresh_board(board_win, board, gameover);
		} else if (!board_can_slide(board)) { //didn't slide, check if game's over
			gameover = true;
			refresh_board(board_win, board, gameover);
			refresh_score(score_win, score, points, max_score);
		}
		flushinp();
	}

	save_game(board, score, max_score);
	endwin();
	return 0;
}
