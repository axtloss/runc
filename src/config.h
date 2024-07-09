#include <stdlib.h>
struct compile_cmd_t {
  char *cmd;
  char *args;
  char *fileext;
};

void compile_cmd_free (struct compile_cmd_t *self);

size_t get_commands(struct compile_cmd_t ***ccmds_glob);
