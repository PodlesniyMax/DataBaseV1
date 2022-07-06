#ifndef DB_TYPES_T

#define DB_TYPES_T

#include <inttypes.h>

#define ID_SIZE 60

enum command {
	INIT,
	ADD,
	QUERY,
	LIST,
	REMOVE
};

struct db_conf {
	uint8_t start_position; /* Position of first record in the file */
	uint8_t record_size;
	uint32_t count_records;
};

struct record {
	char id[ID_SIZE];
	uint32_t count;
	uint32_t left;
	uint32_t right;
};

#endif
