#ifndef COMMAND_T
#define COMMAND_T

void init_db(const char *filename);
void remove_db(const char *filename);
void add_record(const char *filename, const char *id);
void query_record(const char *filename, const char *id);
void list_records(struct input inp);
void load_file(struct input inp);

#endif
