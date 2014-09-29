#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "field.h"

static int  move_line(int line[5], int moves[5]);
static void rotate_l(int dest[5][5], int source[5][5]);
static void rotate_r(int dest[5][5], int source[5][5]);
static void rotate_2(int dest[5][5], int source[5][5]);


static char save_file[256];


int add_rand_square(int field[5][5], int num)
{
	int emptyx[16], emptyy[16], empty_n = 0;
	int x, y;
 // 12.5% chance of getting '4'
	if (num == 0) num = (rand() % 8 == 7) ? 4 : 2;

	for (int y = 1; y <= 4; y++) {
		for (int x = 1; x <= 4; x++) {
			if (field[y][x] == 0) {
				emptyx[empty_n] = x;
				emptyy[empty_n++] = y;
			}
		}
	}

	if (empty_n > 0) {
		int r = rand() % empty_n;
		x = emptyx[r];
		y = emptyy[r];
		field[y][x] = num;
	}
	return empty_n;
}

void start_field(int field[5][5])
{
    memset(field, 0, 25*sizeof(int));
    add_rand_square(field, 2);
    add_rand_square(field, 2);
}

		
int move_field(int field[5][5], int new_field[5][5], int moves[5][5],  int dir)
{
	/* returns points or -1 if didn't move */

	int field_rot[5][5];

	// rotate field
	if (dir == 'l')      copy_field(field_rot, field);
	else if (dir == 'r') rotate_2(field_rot, field);
	else if (dir == 'u') rotate_l(field_rot, field);
	else if (dir == 'd') rotate_r(field_rot, field);
	else exit(1);

	int moved = 0, points = 0, line_points;
	int moves_rot[5][5];
	for (int y = 1; y <= 4; y++) {   // move squares line by line
		line_points = move_line(field_rot[y], moves_rot[y]);
		if (line_points >= 0) {
			moved = 1;
			points += line_points;
		}
	}
	if (!moved) return -1;

	// rotate back
	if (dir == 'l') {
		copy_field(new_field, field_rot);
		copy_field(moves, moves_rot);
	} else if (dir == 'r') {
		rotate_2(new_field, field_rot);
		rotate_2(moves, moves_rot);
	} else if (dir == 'u') {
		rotate_r(new_field, field_rot);
		rotate_r(moves, moves_rot);
	} else if (dir == 'd') {
		rotate_l(new_field, field_rot);
		rotate_l(moves, moves_rot);
	}
	return points;
}

static void rotate_l(int dest[5][5], int source[5][5])
{
	for (int y = 1; y <= 4; y++) {
		for (int x = 1; x <= 4; x++)
			dest[5-x][y] = source[y][x];
	}
}
static void rotate_r(int dest[5][5], int source[5][5])
{
	for (int y = 1; y <= 4; y++) {
		for (int x = 1; x <= 4; x++)
			dest[x][5-y] = source[y][x];
	}
}
static void rotate_2(int dest[5][5], int source[5][5])
{
	for (int y = 1; y <= 4; y++) {
		for (int x = 1; x <= 4; x++)
			dest[y][5-x] = source[y][x];
	}
}

static int move_line(int line[5], int moves[5])
{
	/* returns points or -1 if didn't move,
	   stores moves for animove() */

	int linecp[5], points = 0;
	memcpy(linecp, line, 5*sizeof(int));
	memset(moves, 0, 5*sizeof(int));

	for (int i = 1; i <= 3; i++) {
		if (line[i] == 0) { // found an empty spot, move next square here
			int next;
			for (next = i+1; next <= 4 && line[next] == 0; next++);
			if (next < 5) {
				line[i] = line[next];
				line[next] = 0;
				moves[next] = next - i;
			}
		}
		if (line[i] != 0) { // search for square with same num
			int next;
			for (next = i+1; next <= 4 && line[next] == 0; next++);
			if (next < 5 && line[i] == line[next]) {
				line[i] *= 2;
				points = line[i];
				line[next] = 0;
				moves[next] = next - i;
			}
		}
	}

	if (!memcmp(line, linecp, 5*sizeof(int))) return -1;
	return points;
}


int is_gameover(int field[5][5])
{
	int fcopy[5][5], dummy1[5][5], dummy2[5][5];
	copy_field(fcopy, field);
	if (move_field(fcopy, dummy1, dummy2, 'u') >= 0 ||
		move_field(fcopy, dummy1, dummy2, 'd') >= 0 ||
		move_field(fcopy, dummy1, dummy2, 'l') >= 0 ||
		move_field(fcopy, dummy1, dummy2, 'r') >= 0) {
		return 0;
	}
	return 1;
}

static int get_home(void)
{
	char *home = getenv("HOME");
	if (!home) return 0;

	strncpy(save_file, getenv("HOME"), 249);
    strcat(save_file, "/.2048"); 
    return 1;
}

int save_field(int field[5][5], int score, int max_score)
{
    FILE *fout;
    if (!(fout = fopen(save_file, "w"))) return 0;

	fprintf(fout, "%d\n%d\n", score, max_score);
    for (int y = 1; y <= 4; y++) {
        for (int x = 1; x <= 4; x++) {
            fprintf(fout, "%d ", field[y][x]);
        }
        fputs("\n", fout);
    }
    fclose(fout);
    return 1;
}

int load_field(int field[5][5], int *score, int *max_score)
{
    if (!get_home()) return 0;
    
    FILE *fin;
    if (!(fin = fopen(save_file, "r"))) return 0;
   
	fscanf(fin, "%d%d", score, max_score);
	for (int y = 1; y <= 4; y++) {
        for (int x = 1; x <= 4; x++) {
            fscanf(fin, "%d", &field[y][x]);
        }
    }
    fclose(fin);
    return 1;
} 
