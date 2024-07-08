#define _XOPEN_SOURCE 500
#include <stdlib.h>
#include <extlib.h>
#include <ftw.h>
#include <stdio.h>
#include <string.h>
#include "config.h"

struct compile_cmd_t ***ccmds;
size_t ccmds_size = 0;

int parse_config(const char *fpath, const struct stat *sb,
		 int typeflag, struct FTW *ftwbuf)
{
  if (!S_ISREG (sb->st_mode))
    return 0;

  struct compile_cmd_t *ccmd = malloc (sizeof (struct compile_cmd_t));
  struct compile_cmd_t **ccmds_tmp = realloc (*ccmds, sizeof (ccmds)+sizeof (struct compile_cmd_t));
  if (ccmds_tmp)
    *ccmds = ccmds_tmp;

  FILE *cfg = fopen (fpath, "r");

  int CMD_SET = 1;
  int ARGS_SET = 2;
  int FILEEXT_SET = 4;

  int filled = 0;
  while (filled != (CMD_SET|ARGS_SET|FILEEXT_SET)) {
    char *line = strdup ("");
    int tmp_char;
    while ((tmp_char = fgetc (cfg))) {
      char *line_tmp = realloc (line, strlen (line) + 8);
      if (line_tmp) line = line_tmp;
      sprintf (line, "%s%c", line, tmp_char);
      if (tmp_char == '\n' || tmp_char == '\0')
	break;
    }
    char *seperator_fnd = strstr (line, "=");
    if (!seperator_fnd)
      puts ("invalid config line");
    else {
      char *value = strdup (seperator_fnd+1);
      int identifier_len = strlen (line) - strlen (value) - 1;
      char *identifier = malloc (identifier_len + 1);
      strncpy(identifier, line, identifier_len);
      identifier[identifier_len] = '\0';

      if (strcmp (identifier, "compiler") == 0 && !(filled & CMD_SET)) {
	ccmd->cmd = trim (strdup (value), NULL, NULL);
	filled = filled | CMD_SET;
      }
      else if (strcmp (identifier, "args") == 0 && !(filled & ARGS_SET)) {
	ccmd->args = trim (strdup (value), NULL, NULL);
	filled = filled | ARGS_SET;
      }
      else if (strcmp (identifier, "fileext") == 0 && !(filled & FILEEXT_SET)) {
	ccmd->fileext = trim (strdup (value), NULL, NULL);
	filled = filled | FILEEXT_SET;
      }
      else {
        printf ("warn: invalid configuration: %s\n", fpath);
	exit (1);
      }

      free (identifier);
      free (value);
    }
    free (line);
  }
  fclose (cfg);
  ccmds_tmp[ccmds_size] = ccmd;
  ccmds_size += 1;
  return 0;
}

size_t get_commands(struct compile_cmd_t ***ccmds_glob) {
  ccmds = ccmds_glob;
  *ccmds = malloc (sizeof (struct compile_cmd_t));
  char *home = getenv ("HOME");
  if (!home) {
    puts ("warn: HOME not found. Unable to read home configuration files.");
  }
  char **paths = malloc (sizeof (char*)*4);
  paths[0] = strdup ("/etc/runc.d");
  paths[1] = strdup ("/usr/share/runc");
  if (home) {
    paths[2] = malloc (strlen (home)+strlen ("/.local/share/runc")+1);
    sprintf (paths[2], "%s/.local/share/runc", home);
  }
  for (int i = 0; i < 3; i++) {
    if (!paths[i]) break;
    int err = nftw (paths[i], parse_config, 10, FTW_DEPTH|FTW_MOUNT|FTW_PHYS);
    free (paths[i]);
  }
  free (paths);
  return ccmds_size;
}
