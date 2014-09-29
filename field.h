#ifndef FIELD_H
#define FIELD_H

#define copy_field(dest, source) (memcpy((dest), (source), 25*sizeof(int)))

void start_field(int field[5][5]);
int  add_rand_square(int field[5][5], int num);

int move_field(int field[5][5], int new_field[5][5], int moves[5][5],  int dir);
int is_gameover(int field[5][5]);

int save_field(int field[5][5], int score, int max_score);
int load_field(int field[5][5], int *score, int *max_score);


#endif
