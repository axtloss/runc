#define _XOPEN_SOURCE 500
#include <stdlib.h>
#include <extlib.h>
#include <ftw.h>
#include <stdio.h>
#include <string.h>
#include "config.h"

struct compile_cmd_t ***ccmds;
size_t ccmds_size = 0;

struct compile_cmd_t *compile_cmd_init()
{
  struct compile_cmd_t *self = malloc (sizeof (struct compile_cmd_t));
  self->cmd = NULL;
  self->args = strdup ("");
  self->fileext = NULL;
  return self;
}

void
compile_cmd_free (struct compile_cmd_t *self)
{
  if (self->cmd) free (self->cmd);
  if (self->args) free (self->args);
  if (self->fileext) free (self->fileext);
  free (self);
}

int
parse_config(const char *fpath, const struct stat *sb,
	     int typeflag, struct FTW *ftwbuf)
{
  if (!S_ISREG (sb->st_mode))
    return 0;
  if (sb->st_size < 12) // config file that doesnt even include "compiler=xx" can be ignored
    return 0;

  struct compile_cmd_t *ccmd = compile_cmd_init();
  struct compile_cmd_t **ccmds_tmp = realloc (*ccmds, sizeof (ccmds)+sizeof (struct compile_cmd_t));
  if (ccmds_tmp)
    *ccmds = ccmds_tmp;

  FILE *cfg = fopen (fpath, "r");

  int CMD_SET = 1;
  int FILEEXT_SET = 2;

  int filled = 0;
  while (filled != (CMD_SET|FILEEXT_SET)) {
    char *line = strdup ("");
    int tmp_char = '\0';
    while ((tmp_char = fgetc (cfg))) {
      char *line_tmp = realloc (line, strlen (line) + 8);
      if (line_tmp) line = line_tmp;
      sprintf (line, "%s%c", line, tmp_char);
      if (tmp_char == '\n' || tmp_char == '\0')
	break;
    }
    char *seperator_fnd = strstr (line, "=");
    if (!seperator_fnd)
      fprintf (stderr, "warn: invalid configuration: %s\n", fpath);
    else {
      char *value = strdup (seperator_fnd+1);
      int identifier_len = strlen (line) - strlen (value) - 1;
      char *identifier = malloc (identifier_len + 1);
      strncpy(identifier, line, identifier_len);
      identifier[identifier_len] = '\0';
      if (strcmp (identifier, "compiler") == 0 && !(filled & CMD_SET)) {
	ccmd->cmd = trim (value, NULL, NULL);
	filled = filled | CMD_SET;
      }
      else if (strcmp (identifier, "args") == 0) {
	free (ccmd->args);
	ccmd->args = trim (value, NULL, NULL);
      }
      else if (strcmp (identifier, "fileext") == 0 && !(filled & FILEEXT_SET)) {
	ccmd->fileext = trim (value, NULL, NULL);
	filled = filled | FILEEXT_SET;
      }
      else {
        fprintf (stderr, "warn: invalid configuration: %s\n", fpath);
	free (identifier);
	free (value);
	free (line);
	fclose (cfg);
	free (ccmd);
        return 1;
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
    fprintf (stderr, "warn: HOME not found. Unable to read home configuration files.\n");
  }
  char *paths[2] = {"/etc/runc.d", NULL};
  if (home) {
    paths[1] = malloc (strlen (home)+strlen ("/.config/runc")+1);
    sprintf (paths[1], "%s/.config/runc", home);
  }
  for (int i = 0; i < 2; i++) {
    if (!paths[i]) break;
    nftw (paths[i], parse_config, 10, FTW_DEPTH|FTW_MOUNT|FTW_PHYS);
    if (i==1) free (paths[i]);
  }
  return ccmds_size;
}
