#ifndef VALIDATE_INPUT_T

#define VALIDATE_INPUT_T

enum list_param {
	LIST_START = 0,
	LIST_COUNT = 10
};

struct input validate_input(int argc, char **argv);

#endif
