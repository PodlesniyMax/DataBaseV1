objects = main.o validate_input.o command.o

prog: $(objects)
	cc -g -Wall -o prog $(objects)

main.o: main.c validate_input.h db_types.h
	cc -g -Wall -c main.c
validate_input.o: validate_input.c validate_input.h db_types.h
	cc -g -Wall -c validate_input.c
command.o: command.c command.h db_types.h
	cc -g -Wall -c command.c

clean:
	rm prog $(objects)
