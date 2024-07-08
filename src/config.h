#include <stdlib.h>
struct compile_cmd_t {
  char *cmd;
  char *args;
  char *fileext;
};

size_t get_commands(struct compile_cmd_t ***ccmds_glob);
