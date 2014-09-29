#define _GNU_SOURCE
#include <ncurses.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "draw.h"
#include "field.h"


int main(void)
{
	prepare_screen();
    srand(time(NULL));

    int field[5][5];
	int score, max_score, gameover = 0;
    if (!load_field(field, &score, &max_score)) {
		start_field(field);
		score = max_score = 0;
    }

	WINDOW *win = NULL, *score_win = NULL;
	init_win(&win, &score_win);

    draw_field(win, field, 0);
	draw_score(score_win, 0, score, max_score);	
	wrefresh(win);
	wrefresh(score_win);

	int ch;
	const struct timespec addsquare = {.tv_sec = 0, .tv_nsec = 100000000};

	while ((ch = wgetch(win)) != 'q' && ch != 'Q') {  // q to quit
		int dir, points;
		int new_field[5][5] = {};
		int moves[5][5] = {};

		switch(ch) {
            case KEY_UP:    dir = 'u'; break; // moving
            case KEY_DOWN:  dir = 'd'; break;
            case KEY_LEFT:  dir = 'l'; break;
            case KEY_RIGHT: dir = 'r'; break;

            case 'r': case 'R':               // start new game
				gameover = score = 0;	
                start_field(field);
				draw_field(win, field, 0);
				draw_score(score_win, 0, score, max_score);
				wrefresh(win);
				wrefresh(score_win);
                continue;
                break;

			case KEY_RESIZE:                  // terminal resize	
				init_win(&win, &score_win);
				draw_field(win, field, gameover);
				draw_score(score_win, 0, score, max_score);
				wrefresh(win);
				wrefresh(score_win);
				continue;
				break;

			default: continue; // main loop
        }

		if (gameover) continue;

		points = move_field(field, new_field, moves, dir);
        if (points >= 0) {
			draw_score(score_win, points, score, max_score);
			wrefresh(score_win);
            animove(win, field, moves, dir);
            copy_field(field, new_field);

			score += points;
			if (score > max_score) max_score = score;
            draw_field(win, field, 0);
			draw_score(score_win, points, score, max_score);
			wrefresh(win);
			wrefresh(score_win);
			
			nanosleep(&addsquare, NULL);
            add_rand_square(field, 0);
            draw_field(win, field, 0);
			wrefresh(win);
        } else if (is_gameover(field)) { //didn't move, check if game's over
			draw_field(win, field, 1);
			draw_score(score_win, 0, score, max_score);
			wrefresh(win);
			wrefresh(score_win);
			gameover = 1;
		}
		flushinp();
    }
    
    save_field(field, score, max_score);
    endwin();
    return 0;
}

