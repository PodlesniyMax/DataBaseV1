#include <stdio.h>
#include <stdlib.h>

#include "db_types.h"
#include "validate_input.h"
#include "command.h"

int main(int argc, char **argv)
{
	struct input *inp = NULL;
	inp = validate_input(argc, argv);
	if (inp->error) {
		fprintf(stderr, "%s", inp->error);
		fprintf(stderr, "Usage: %s filename command [identifier]\n", argv[0]);
		fprintf(stderr, "\tMax length of identifier is %d\n", ID_SIZE - 1);
		free(inp->error);
		free(inp);
		exit(EXIT_FAILURE);
	}

	switch (inp->command) {
		case INIT:
			init_db(inp->filename);
			break;
		case REMOVE:
			remove_db(inp->filename);
			break;
		case ADD:
			add_record(inp->filename, inp->id);
			break;
	}

	free(inp);
	return 0;
}
