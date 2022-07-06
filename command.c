#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "command.h"

void init_db(const char *filename)
{
	int fd, oflag;
	mode_t mode;

	oflag = O_WRONLY|O_CREAT|O_EXCL;
	mode = S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH;
	fd = open(filename, oflag, mode);
	if (fd == -1) {
		perror(filename);
		exit(EXIT_FAILURE);
	}
	close(fd);
}

void remove_db(const char *filename)
{
	if (unlink(filename) == -1) {
		perror(filename);
		exit(EXIT_FAILURE);
	}
}
