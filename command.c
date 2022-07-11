#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <inttypes.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "db_types.h"
#include "command.h"

static int search_record(int fd, const char *id);
static void add_new_record(int fd, const char *id);
static void update_db_config(int fd, struct db_conf conf);
static void update_record(int fd, int rec_pos);
static int tree_search(int fd, int pos, const char *id);
static void mount_record_to_tree(int fd, int rec_pos, const char *id);
static int search_parent(int fd, int pos, const char *id);

void init_db(const char *filename)
{
	int fd, oflag;
	mode_t mode;
	struct db_conf conf; /* => db_types.h */

	oflag = O_WRONLY|O_CREAT|O_EXCL;
	mode = S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH;
	fd = open(filename, oflag, mode);
	if (fd == -1) {
		perror(filename);
		exit(EXIT_FAILURE);
	}

	/* first record is located just after config information*/
	conf.start_position = (uint8_t)sizeof(struct db_conf);

	conf.record_size = (uint8_t)sizeof(struct record);
	conf.count_records = 0;

	write(fd, &conf, sizeof(conf));
	close(fd);
}

void remove_db(const char *filename)
{
	if (unlink(filename) == -1) {
		perror(filename);
		exit(EXIT_FAILURE);
	}
}

void add_record(const char *filename, const char *id, int filedesc)
{
	struct db_conf conf;
	int fd, rec_pos;
	size_t size;

	if (filename) {
		fd = open(filename, O_RDWR);
	} else {
		fd = filedesc;
	}

	if (fd == -1 && filename) {
		perror(filename);
		exit(EXIT_FAILURE);
	}

	/* read information about file configuration */
	lseek(fd, 0, SEEK_SET);
	size = read(fd, &conf, sizeof(conf));
	if (size != sizeof(conf)) {
		fprintf(stderr, "%s: fail read operation\n", filename);
		exit(EXIT_FAILURE);
	}

	/* check if id is already in DB */
	rec_pos = search_record(fd, id);

	if (rec_pos == -1) {
		add_new_record(fd, id);
		conf.count_records++;
		update_db_config(fd, conf);
	} else {
		update_record(fd, rec_pos);
	}

	if (filename) {
		close(fd);
	}
}

void query_record(const char *filename, const char *id)
{
	struct record rec;
	int fd, rec_pos, count = 0;

	fd = open(filename, O_RDONLY);
	if (fd == -1) {
		perror(filename);
		exit(EXIT_FAILURE);
	}
	rec_pos = search_record(fd, id);
	if (rec_pos != -1) {
		lseek(fd, rec_pos, SEEK_SET);
		read(fd, &rec, sizeof(rec));
		count = rec.count;
	}

	printf("Identifier: %s\nCount: %d\n", id, count);
	close(fd);
}

void list_records(struct input inp)
{
	struct db_conf conf;
	struct record rec;
	int fd, pos, buff_size, count, i;
	uint8_t *buff;

	buff_size = 100 * sizeof(rec);
	buff = (uint8_t *)malloc(buff_size);

	fd = open(inp.filename, O_RDONLY);
	if (fd == -1) {
		perror(inp.filename);
		exit(EXIT_FAILURE);
	}
	read(fd, &conf, sizeof(conf));
	if (conf.count_records == 0) {
		free(buff);
		close(fd);
		return;
	}
	pos = (int)conf.start_position + (inp.list_start * (int)conf.record_size);
	lseek(fd, pos, SEEK_SET);
	while (inp.list_count > 0) {
		if (buff_size > inp.list_count * sizeof(rec)) {
			buff_size = inp.list_count * sizeof(rec);
		}
		count = read(fd, buff, buff_size);
		inp.list_count -= count;
		for (i = 0; i < count; i += sizeof(rec)) {
			memcpy(&rec, buff + i, sizeof(rec));
			printf("Identifier: %s\nCount: %d\n\n", rec.id, rec.count);
		}
	}
	close(fd);
}

void load_file(struct input inp)
{
	int fd_dest, fd_src, size, state, id_pos, buff_pos, buff_size = 4096;
	uint8_t *buff;
	uint8_t id[ID_SIZE];
	enum { OUT, IN };

	fd_dest = open(inp.filename, O_RDWR);
	if (fd_dest == -1) {
		perror(inp.filename);
		exit(EXIT_FAILURE);
	}

	fd_src = open(inp.load_filename, O_RDONLY);
	if (fd_src == -1) {
		perror(inp.load_filename);
		exit(EXIT_FAILURE);
	}

	buff = (uint8_t *)malloc(buff_size);
	state = OUT;
	while ((size = read(fd_src, buff, buff_size)) > 0) {
		for (buff_pos = 0; buff_pos < size; buff_pos++) {
			if (buff[buff_pos] == '\n' && state == IN) {
				state = OUT;
				id[id_pos] = '\0';
				id_pos = 0;
				add_record(NULL, (char *)id, fd_dest);
				continue;
			} else if (id_pos == ID_SIZE - 1) {
				continue;
			} else {
				state = IN;
				id[id_pos] = buff[buff_pos];
				id_pos++;
			}
		}
	}
	free(buff);
	close(fd_dest);
	close(fd_src);
}

static int search_record(int fd, const char *id)
{
	struct db_conf conf;

	lseek(fd, 0, SEEK_SET);
	read(fd, &conf, sizeof(conf));

	/* is DB empty ? */
	if (conf.count_records == 0) {
		return -1; 
	}

	return tree_search(fd, conf.start_position, id);
}

static void add_new_record(int fd, const char *id)
{
	struct record rec;
	int rec_pos, size;

	for (int i = 0; i < ID_SIZE; i++) {
		rec.id[i] = 0;
	}
	strcpy(rec.id, id);
	rec.count = 1;
	rec.left = 0;
	rec.right = 0;

	rec_pos = lseek(fd, 0, SEEK_END);
	if (rec_pos == -1) {
		perror("In function add_new_record: ");
		exit(EXIT_FAILURE);
	}

	size = write(fd, &rec, sizeof(rec));
	if (size != sizeof(rec)) {
		fprintf(stderr, "Fail operation\n");
		ftruncate(fd, rec_pos);       /* restore file */
		exit(EXIT_FAILURE);
	}

	mount_record_to_tree(fd, rec_pos, id);
}

static void update_db_config(int fd, struct db_conf conf)
{
	lseek(fd, 0, SEEK_SET);
	write(fd, &conf, sizeof(conf));
}

static void update_record(int fd, int rec_pos)
{
	struct record rec;

	lseek(fd, rec_pos, SEEK_SET);
	read(fd, &rec, sizeof(rec));
	rec.count++;
	lseek(fd, rec_pos, SEEK_SET);
	write(fd, &rec, sizeof(rec));
}

static int tree_search(int fd, int pos, const char *id)
{
	struct record rec;
	int state;

	if (lseek(fd, (off_t)pos, SEEK_SET) == -1) {
		perror("In function tree_search:");
		exit(EXIT_FAILURE);
	}

	read(fd, &rec, sizeof(rec));

	state = strcmp(id, rec.id);
	if (state == 0) {
		return pos;
	} else if (state < 0 && rec.left) {
		return tree_search(fd, rec.left, id);
	} else if (state > 0 && rec.right) {
		return tree_search(fd, rec.right, id);
	} else {
		return -1;
	}
}

static void mount_record_to_tree(int fd, int rec_pos, const char *id)
{
	struct db_conf conf;
	struct record rec;
	int parent_pos;

	lseek(fd, 0, SEEK_SET);
	read(fd, &conf, sizeof(conf));

	if (rec_pos == conf.start_position) {
		return;
	}
	parent_pos = search_parent(fd, conf.start_position, id);
	lseek(fd, parent_pos, SEEK_SET);
	read(fd, &rec, sizeof(rec));
	if (strcmp(id, rec.id) < 0) {
		rec.left = rec_pos;
	} else {
		rec.right = rec_pos;
	}
	lseek(fd, parent_pos, SEEK_SET);
	write(fd, &rec, sizeof(rec));
}

static int search_parent(int fd, int pos, const char *id)
{
	struct record rec;
	int state;

	lseek(fd, pos, SEEK_SET);
	read(fd, &rec, sizeof(rec));

	state = strcmp(id, rec.id);
	if (state < 0 && rec.left) {
		return search_parent(fd, rec.left, id);
	} else if (state >= 0 && rec.right) {
		return search_parent(fd, rec.right, id);
	} else {
		return pos;
	}
}
