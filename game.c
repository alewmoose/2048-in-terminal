#define _GNU_SOURCE
#include <unistd.h>
#include <ncurses.h>

#include <stdlib.h>
#include <signal.h>
#include <setjmp.h>
#include <time.h>
#include <stdbool.h>

#include "draw.h"
#include "board.h"
#include "save.h"
#include "logit.h"


#define ESC_KEY 27
static jmp_buf  jmpbuf;
static sigset_t all_signals;


/* Not sure if it's safe to longjmp from signal handler.
 * Jumps back to main on every signal.
 */
static void sig_handler(int __attribute__((unused))sig_no)
{
	sigprocmask(SIG_BLOCK, &all_signals, NULL);
	longjmp(jmpbuf, 1);
}



int main(void)
{
	if (!isatty(fileno(stdout)) ||
	    !isatty(fileno(stdin))) {
		exit(1);
	}

	sigfillset(&all_signals);
	signal(SIGINT,  sig_handler);
	signal(SIGABRT, sig_handler);
	signal(SIGTERM, sig_handler);
	signal(SIGHUP,  sig_handler);

	Board board;
	Stats stats = {.game_over = 0, .auto_save = true};
	/* use volatile to supress compiler warning:
	 * "variable might be clobbered by longjmp" */
	const struct timespec addtile_time = {.tv_sec = 0,
		                                .tv_nsec = 100000000};
	srand(time(NULL));


	sigprocmask(SIG_BLOCK, &all_signals, NULL);
	if (lock_save_file() != 0) {
		stats.auto_save = false;
	}
	if (load_game(&board, &stats) != 0) {
		board_start(&board);
		stats.score = 0;
		stats.max_score = 0;
	}
	sigprocmask(SIG_UNBLOCK, &all_signals, NULL);
	logit("%d\n", sizeof(bool));

	setup_screen();
	volatile bool terminal_too_small = false;
	if (init_win() == WIN_TOO_SMALL) {
		terminal_too_small = true;
		print_too_small();
	} else {
		draw(&board, &stats);
	}


	if (setjmp(jmpbuf) != 0) {
		/* longjmp from sig_handler */
		goto sigint;
	}




	int ch;
	while ((ch = getch()) != 'q' && ch != 'Q' && ch != ESC_KEY) {
		/* if terminal's too small do nothing
		   until it's restored */
		if (terminal_too_small && ch != KEY_RESIZE)
			continue;
		Dir dir;
		Board new_board;
		Board moves;

		switch(ch) {
		case KEY_UP:    dir = UP;    break;
		case KEY_DOWN:  dir = DOWN;  break;
		case KEY_LEFT:  dir = LEFT;  break;
		case KEY_RIGHT: dir = RIGHT; break;

		/* restart */
		case 'r': case 'R':
			stats.score = 0;
			stats.game_over = false;
			board_start(&board);
			draw(&board, &stats);
			continue;

		/* terminal resize */
		case KEY_RESIZE:
			if (init_win() == WIN_TOO_SMALL) {
				terminal_too_small = true;
				print_too_small();
			} else {
				terminal_too_small = false;
				draw(&board, &stats);
			}
			continue;
		default:
			continue;
		}

		if (stats.game_over) continue;

		/* block all signals while operating on board */
		sigprocmask(SIG_BLOCK, &all_signals, NULL);

		stats.points = board_slide(&board, &new_board, &moves, dir);

		if (stats.points >= 0) {
			draw(NULL, &stats);
			draw_slide(&board, &moves, dir);

			board_copy(&board, &new_board);
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
		sigprocmask(SIG_UNBLOCK, &all_signals, NULL);
		flushinp();
	}
	/* block all signals before saving */
	sigprocmask(SIG_BLOCK, &all_signals, NULL);
sigint:
	endwin();
	save_game(&board, &stats);
	return 0;
}
