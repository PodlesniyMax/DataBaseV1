#ifndef VALIDATE_INPUT_T

#define VALIDATE_INPUT_T

struct input {
	char *filename,
         *id,
		 *error;
	int command;
};

struct input *validate_input(int argc, char **argv);

#endif
