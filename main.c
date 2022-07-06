#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <inttypes.h>

#include "db_types.h"
#include "validate_input.h"

int main(int argc, char **argv)
{
	struct input *inp = NULL;
	inp = validate_input(argc, argv);
	if (inp->error) {
		fprintf(stderr, "%s", inp->error);
		fprintf(stderr, "Usage: %s filename command [identifier]\n", argv[0]);
		free(inp->error);
		free(inp);
		exit(EXIT_FAILURE);
	}
	free(inp);
	return 0;
}
