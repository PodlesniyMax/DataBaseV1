#include <stdio.h>
#include <stdlib.h>

#include "db_types.h"
#include "validate_input.h"
#include "command.h"

int main(int argc, char **argv)
{
	struct input inp;

	inp = validate_input(argc, argv);

	switch (inp.command) {
		case INIT:
			init_db(inp.filename);
			break;
		case REMOVE:
			remove_db(inp.filename);
			break;
		case ADD:
			add_record(inp.filename, inp.id);
			break;
		case QUERY:
			query_record(inp.filename, inp.id);
			break;
		case LIST:
			list_records(inp);
			break;
	}

	return 0;
}
