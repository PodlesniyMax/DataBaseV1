#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "db_types.h"
#include "validate_input.h"

static int get_command(const char *command);

struct input *validate_input(int argc, char **argv)
{
	struct input *inp = NULL;
	char *err = NULL;
	int err_size = 128;

	err = malloc(sizeof(char) * err_size);
	if (!err) {
		perror("In function validate_input");
		exit(EXIT_FAILURE);
	}

	inp = malloc(sizeof(struct input));
	if (!inp) {
		perror("In function validate_input");
		exit(EXIT_FAILURE);
	}

	inp->filename = NULL;
	inp->id = NULL;
	inp->error = NULL;
	inp->command = -1;

	if (argc < 3) {
		strcpy(err, "Too few arguments\n");
		inp->error = err;
		return inp;
	}

	inp->filename = argv[1];
	inp->command = get_command(argv[2]);
	if (inp->command == -1) {
		strcpy(err, "Wrong command\n");
		inp->error = err;
		return inp;
	}

	if (inp->command == ADD || inp->command == QUERY) {
		if (argc < 4) {
			strcpy(err, "Too few arguments\n");
			inp->error = err;
			return inp;
		}
		inp->id = argv[3];
	}

	free(err);
	return inp;
}

static int get_command(const char *command)
{
	int cmd;

	if (strcmp(command, "init") == 0) {
		cmd = INIT;
	} else if (strcmp(command, "add") == 0) {
		cmd = ADD;
	} else if (strcmp(command, "query") == 0) {
		cmd = QUERY;
	} else if (strcmp(command, "list") == 0) {
		cmd = LIST;
	} else if (strcmp(command, "remove") == 0) {
		cmd = REMOVE;
	} else {
		cmd = -1;
	}

	return cmd;
}
