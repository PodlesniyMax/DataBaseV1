#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "db_types.h"
#include "validate_input.h"

static int get_command(const char *command);

struct input validate_input(int argc, char **argv)
{
	struct input inp;

	inp.filename = NULL;
	inp.id = NULL;
	inp.load_filename = NULL;
	inp.command = -1;
	inp.list_start = LIST_START;
	inp.list_count = LIST_COUNT;

	if (argc < 3) {
		fprintf(stderr, "Too few arguments\n");
		exit(EXIT_FAILURE);
	}

	inp.filename = argv[1];
	inp.command = get_command(argv[2]);
	if (inp.command == -1) {
		fprintf(stderr, "Wrong command\n");
		exit(EXIT_FAILURE);
	}

	if (inp.command == ADD || inp.command == QUERY) {
		if (argc < 4) {
			fprintf(stderr, "Too few arguments\n");
			exit(EXIT_FAILURE);
		}
		inp.id = argv[3];
	}

	if (inp.command == LOAD) {
		if (argc < 4) {
			fprintf(stderr, "Too few arguments\n");
			exit(EXIT_FAILURE);
		}
		inp.load_filename = argv[3];
	}

	if (inp.id && strlen(inp.id) > ID_SIZE - 1) {
		fprintf(stderr, "Too long identifier\n");
		exit(EXIT_FAILURE);
	}

	if (inp.command == LIST) {
		if (argc > 3) {
			inp.list_start = atoi(argv[3]);
		}
		if (argc > 4) {
			inp.list_count = atoi(argv[4]);
		}
	}

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
	} else if (strcmp(command, "load") == 0) {
		cmd = LOAD;
	} else {
		cmd = -1;
	}

	return cmd;
}
