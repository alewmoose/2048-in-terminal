#ifndef COMMON_H
#define COMMON_H

#define BOARD_SIZE  4
#define BOARD_TILES (BOARD_SIZE * BOARD_SIZE)

/* Each tile is represented as power of two,
 * empty tile is 0 */
typedef struct board {
	int tiles[BOARD_SIZE][BOARD_SIZE];
} Board;

typedef struct stats {
	int score;
	int points; /* points for the last slide */
	int max_score;
	bool game_over;
	bool auto_save;
} Stats;

typedef struct coord {
	int x, y;
} Coord;

typedef enum dir {UP, DOWN, LEFT, RIGHT} Dir;

#endif
