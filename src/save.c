#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/file.h>
#include "common.h"
#include "save.h"


#define PATH_LEN 256

#define MAX_POSSIBLE_SCORE  3932156
#define MAX_POSSIBLE_TILE   17

static char filename[PATH_LEN] = "";
static int  fd = -1;

static bool sane(Board *board, Stats *stats);
static int  get_filename(void);


int load_game(Board *board, Stats *stats)
{
	stats->auto_save = false;

	if (get_filename() == -1)
		return -1;

	fd = open(filename, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
	if (fd == -1)
		return -1;

	if (flock(fd, LOCK_EX | LOCK_NB) != -1)
		stats->auto_save = true;

	ssize_t s;
	s = read(fd, &stats->score, sizeof(int));
	if (s != sizeof(int))
		goto read_failed;

	s = read(fd, &stats->max_score, sizeof(int));
	if (s != sizeof(int))
		goto read_failed;

	s = read(fd, board, sizeof(Board));
	if (s != sizeof(Board))
		goto read_failed;

	if (!sane(board, stats))
		goto read_failed;

	return 0;

read_failed:
	if (!stats->auto_save)
		close(fd);
	return -1;
}

int save_game(const Board *board, const Stats *stats)
{
	if (fd == -1 || !stats->auto_save)
		return -1;

	off_t off;
	off = lseek(fd, 0, SEEK_SET);
	if (off == -1)
		return -1;

	ssize_t s;
	s = write(fd, &stats->score,     sizeof(int));
	if (s != sizeof(int))
			goto err;

	s = write(fd, &stats->max_score, sizeof(int));
	if (s != sizeof(int))
			goto err;

	s = write(fd, board, sizeof(Board));
	if (s != sizeof(Board))
			goto err;

	/* game is saved only once, close the file */
err:
	close(fd);
	return s == -1 ? -1 : 0;
}

static int get_filename(void)
{
	char *home = getenv("HOME");
	if (!home || strlen(home) > PATH_LEN-7)
		return -1;

	strcpy(filename, home);
	strcat(filename, "/.2048");
	return 0;
}

static bool sane(Board *board, Stats *stats)
{
	if (stats->score < 0 || stats->max_score < 0 ||
	    stats->max_score > MAX_POSSIBLE_SCORE ||
	    stats->score > stats->max_score)
		return false;

	for (int y = 0; y < BOARD_SIZE; y++) {
		for (int x = 0; x < BOARD_SIZE; x++) {
			int tile = board->tiles[y][x];
			if (tile > MAX_POSSIBLE_TILE || tile < 0)
				return false;
		}
	}
	return true;
}
