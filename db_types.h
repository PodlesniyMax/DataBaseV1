#ifndef DB_TYPES_T

#define DB_TYPES_T

#include <inttypes.h>

enum command {
	INIT,
	ADD,
	QUERY,
	LIST,
	REMOVE
};

struct db_conf {
	uint8_t start_position;
};

#endif
