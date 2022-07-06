#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <inttypes.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "db_types.h"
#include "command.h"

void init_db(const char *filename)
{
	int fd, oflag;
	mode_t mode;
	struct db_conf conf;

	oflag = O_WRONLY|O_CREAT|O_EXCL;
	mode = S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH;
	fd = open(filename, oflag, mode);
	if (fd == -1) {
		perror(filename);
		exit(EXIT_FAILURE);
	}
	conf.start_position = (uint8_t)sizeof(struct db_conf);
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
