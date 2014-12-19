#ifndef COMMON_H
#define COMMON_H

#define BOARD_SIZE  4
#define BOARD_TILES (BOARD_SIZE * BOARD_SIZE)

typedef int board_t[BOARD_SIZE][BOARD_SIZE];
typedef enum {UP, DOWN, LEFT, RIGHT} dir_t;


#endif
