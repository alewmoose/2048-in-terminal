#define _GNU_SOURCE
#include <ncurses.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "draw.h"
#include "board.h"


int main(void)
{
	setup_screen();
    srand(time(NULL));
	const struct timespec addsquare_time = {.tv_sec = 0, .tv_nsec = 100000000};

    board_t board;
	int score, max_score, gameover = 0;
    if (!load_game(board, &score, &max_score)) {
		board_start(board);
		score = max_score = 0;
    }

	WINDOW *board_win = NULL, *score_win = NULL;
	if (!init_win(&board_win, &score_win)) {
		endwin();
		exit(1);
	}

    draw_board(board_win, board, 0);
	draw_score(score_win, score, 0, max_score);	
	wrefresh(board_win);
	wrefresh(score_win);

	int ch;
	while ((ch = wgetch(board_win)) != 'q' && ch != 'Q') {  // q to quit
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
				draw_board(board_win, board, 0);
				draw_score(score_win, score, 0, max_score);
				wrefresh(board_win);
				wrefresh(score_win);
                continue;
                break;

			case KEY_RESIZE:                  // terminal resize	
				if (!init_win(&board_win, &score_win)) {
					endwin();
					exit(1);
				}
				draw_board(board_win, board, gameover);
				draw_score(score_win, score, points, max_score);
				wrefresh(board_win);
				wrefresh(score_win);
				continue;
				break;

			default: continue; // main loop
        }

		if (gameover) continue;

		points = board_slide(board, new_board, moves, dir);
        if (points >= 0) {
			draw_score(score_win, score, points, max_score);
			wrefresh(score_win);
            draw_slide(board_win, board, moves, dir);
            board_copy(board, new_board);

			score += points;
			if (score > max_score) max_score = score;
            draw_board(board_win, board, 0);
			draw_score(score_win, score, points, max_score);
			wrefresh(board_win);
			wrefresh(score_win);
			
			nanosleep(&addsquare_time, NULL);
            board_add_tile(board, 0);
            draw_board(board_win, board, 0);
			wrefresh(board_win);
        } else if (!board_can_slide(board)) { //didn't move, check if game's over
			draw_board(board_win, board, 1);
			draw_score(score_win, score, 0, max_score);
			wrefresh(board_win);
			wrefresh(score_win);
			gameover = 1;
		}
		flushinp();
    }
    
    save_game(board, score, max_score);
    endwin();
    return 0;
} 
