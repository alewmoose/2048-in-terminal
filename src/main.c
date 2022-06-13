#include <unistd.h>
#include <ncurses.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <stdbool.h>
#include "draw.h"
#include "board.h"
#include "save.h"

static sigset_t all_signals;
static Board board;
static Stats stats = {.auto_save = false, .game_over = false};


static void sig_handler(int __attribute__((unused))sig_no)
{
	sigprocmask(SIG_BLOCK, &all_signals, NULL);
	save_game(&board, &stats);
	endwin();
	exit(0);
}


int main(void)
{
	const struct timespec addtile_time = {
		.tv_sec = 0,
		.tv_nsec = 100000000
	};
	bool show_animations = 1;
	bool terminal_too_small;

	if (!isatty(fileno(stdout)) ||
	    !isatty(fileno(stdin))) {
		exit(1);
	}

	srand(time(NULL));

	sigfillset(&all_signals);
	sigdelset(&all_signals, SIGKILL);
	sigdelset(&all_signals, SIGSTOP);
	sigdelset(&all_signals, SIGTSTP);
	sigdelset(&all_signals, SIGCONT);
	signal(SIGINT,  sig_handler);
	signal(SIGABRT, sig_handler);
	signal(SIGTERM, sig_handler);
	signal(SIGHUP,  sig_handler);

	sigprocmask(SIG_BLOCK, &all_signals, NULL);
	if (load_game(&board, &stats) != 0) {
		board_start(&board);
		stats.score = 0;
		stats.max_score = 0;
	}

	setup_screen();
	if (init_win() == WIN_TOO_SMALL) {
		terminal_too_small = true;
		print_too_small();
	} else {
		terminal_too_small = false;
		draw(&board, &stats);
	}

	sigprocmask(SIG_UNBLOCK, &all_signals, NULL);

	int ch;
	while ((ch = getch()) != 'q' && ch != 'Q') {
		Dir dir;
		Board new_board;
		Board moves;

		sigprocmask(SIG_BLOCK, &all_signals, NULL);

		if (terminal_too_small && ch != KEY_RESIZE)
			goto next;

		switch(ch) {
		case KEY_UP: case 'k': case 'K':    dir = UP;    break;
		case KEY_DOWN: case 'j': case 'J':  dir = DOWN;  break;
		case KEY_LEFT: case 'h': case 'H':  dir = LEFT;  break;
		case KEY_RIGHT: case 'l': case 'L': dir = RIGHT; break;

		/* restart */
		case 'r': case 'R':
			stats.score = 0;
			stats.game_over = false;
			board_start(&board);
			draw(&board, &stats);
			goto next;
		
		/* toggle animations */
		case 'a': case 'A':
			show_animations = !show_animations;
			goto next;

		/* terminal resize */
		case KEY_RESIZE:
			if (init_win() == WIN_TOO_SMALL) {
				terminal_too_small = true;
				print_too_small();
			} else {
				terminal_too_small = false;
				draw(&board, &stats);
			}
			goto next;
		default:
			goto next;
		}

		if (stats.game_over)
			goto next;

		stats.points = board_slide(&board, &new_board, &moves, dir);

		if (stats.points >= 0) {
			draw(NULL, &stats); /* show +points */
			if (show_animations)
				draw_slide(&board, &moves, dir);

			board = new_board;
			stats.score += stats.points;
			if (stats.score > stats.max_score)
				stats.max_score = stats.score;
			draw(&board, &stats);

			nanosleep(&addtile_time, NULL);
			board_add_tile(&board, false);
			draw(&board, NULL);
		/* didn't slide, check if game's over */
		} else if (!board_can_slide(&board)) {
			stats.game_over = true;
			draw(&board, &stats);
		}
		flushinp();
	next:
		sigprocmask(SIG_UNBLOCK, &all_signals, NULL);
	}

	/* block all signals before saving */
	sigprocmask(SIG_BLOCK, &all_signals, NULL);
	endwin();

	if (stats.game_over) {
		board_start(&board);
		stats.score = 0;
	}

	save_game(&board, &stats);
	return 0;
}
