#define _GNU_SOURCE
#include <ncurses.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "draw.h"
#include "board.h"

void logit(char *msg)
{
	static FILE *lf = NULL;
	if (!lf) {
		lf = fopen("log", "w");
		if (!lf) {
			endwin();
			exit(1);
		}
	}
	time_t t = time(NULL);
	fprintf(lf, "%s\t%s\n", ctime(&t), msg);
}


int main(void)
{
	setup_screen();
	srand(time(NULL));
	const struct timespec addsquare_time = {.tv_sec = 0, .tv_nsec = 100000000};
	int terminal_too_small = 0;

	board_t board;
	int score, max_score, gameover = 0;
	if (!load_game(board, &score, &max_score)) {
		board_start(board);
		score = max_score = 0;
	}

	WINDOW *board_win = NULL, *score_win = NULL;
	if (init_win(&board_win, &score_win) == WIN_TOO_SMALL) {
		terminal_too_small = 1;
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
		board_t new_board = {};
		board_t moves = {};

		switch(ch) {
			case KEY_UP:    dir = 'u'; break; // moving
			case KEY_DOWN:  dir = 'd'; break;
			case KEY_LEFT:  dir = 'l'; break;
			case KEY_RIGHT: dir = 'r'; break;

			case 'r': case 'R':               // start new game
				gameover = score = 0;
				board_start(board);
				refresh_board(board_win, board, gameover);
				refresh_score(score_win, score, 0, max_score);
				continue;
				break;

			case KEY_RESIZE:                  // terminal resize
				if (init_win(&board_win, &score_win) == WIN_TOO_SMALL) {
					terminal_too_small = 1;
					print_too_small();
					continue;
				} else {
					terminal_too_small = 0;
				}
				refresh_board(board_win, board, gameover);
				refresh_score(score_win, score, points, max_score);
				continue;
				break;
			default:
				logit("other key");
				continue;
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
			board_add_tile(board, 0);
			refresh_board(board_win, board, gameover);
		} else if (!board_can_slide(board)) { //didn't slide, check if game's over
			gameover = 1;
			refresh_board(board_win, board, gameover);
			refresh_score(score_win, score, points, max_score);
		}
		flushinp();
	}

	save_game(board, score, max_score);
	endwin();
	return 0;
}
