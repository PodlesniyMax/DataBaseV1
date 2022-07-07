//TODO update_record in function add_record

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

//#define DEBUG

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

void add_record(const char *filename, const char *id)
{
	struct db_conf conf;
	int fd, rec_pos;
	size_t size;

	fd = open(filename, O_RDWR);

	/* read information about file configuration */
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

	close(fd);
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
	printf("func: update_db_conf\n");
}

static void update_record(int fd, int rec_pos)
{
	struct record rec;

	lseek(fd, rec_pos, SEEK_SET);
	read(fd, &rec, sizeof(rec));
	rec.count++;
	lseek(fd, rec_pos, SEEK_SET);
	write(fd, &rec, sizeof(rec));
	printf("func: update_record\n");
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
	printf("Mount func\n");
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
	printf("search_parent: %s\n", rec.id);

	state = strcmp(id, rec.id);
	if (state < 0 && rec.left) {
		return search_parent(fd, rec.left, id);
	} else if (state >= 0 && rec.right) {
		return search_parent(fd, rec.right, id);
	} else {
		return pos;
	}
}
