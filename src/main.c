#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define _XOPEN_SOURCE 500
#include <extlib.h>
#include "config.h"


struct compile_cmd_t *find_cmd(char *file_extension, struct compile_cmd_t **ccmds, size_t ccmds_len) {
  for (int i = 0; i < ccmds_len; i++) {
    if (strcmp (ccmds[i]->fileext, file_extension) == 0) {
      return ccmds[i];
    }
  }
  return NULL;
}

int main (int argc, char *argv[]) {
  int errno = 0;
  size_t parts_len = 0;
  size_t ccmds_len = 0;
  struct compile_cmd_t **ccmds = NULL;
  struct compile_cmd_t *matching_cmd = NULL;
  char *tmpdir = strdup ("/tmp/XXXXXX");;
  char *compile_file = NULL;
  char *ap = NULL;
  char *cmd = NULL;
  char *excmd = NULL;
  char *fileparts[strlen (argv[argc-1])];

  if (argc < 2) {
    fprintf (stderr, "usage: %s <source file>", argv[0]);
    errno = 1;
    goto EXIT;
  }
  compile_file = strdup(argv[argc-1]);
  ccmds_len = get_commands(&ccmds);

  for (int i = 0; (ap = strsep (&argv[argc-1], ".")); i++) {
    fileparts[i] = ap;
    parts_len += 1;
  }
  if (parts_len == 0) {
    fprintf (stderr, "error: no file extension found\n");
    errno = 1;
    goto EXIT;
  }

  matching_cmd = find_cmd(fileparts[parts_len-1], ccmds, ccmds_len);
  if (!matching_cmd) {
    fprintf (stderr, "error: no complier for file extension '%s' found\n", fileparts[parts_len-1]);
    errno = 1;
    goto EXIT;
  }

  tmpdir = mkdtemp(tmpdir);
  if (!tmpdir) {
    errno = 1;
    goto EXIT;
  }

  cmd = malloc (strlen (matching_cmd->cmd)+
		      strlen (matching_cmd->args)+
		      strlen (compile_file)+
		      strlen ("   -o   ")+
		      strlen (tmpdir)+
		      strlen ("/exec")+1);
  if (!cmd) {
    errno = 1;
    goto EXIT;
  }
  excmd = malloc (strlen (tmpdir)+strlen ("/exec")+1);
  if (!excmd) {
    errno = 1;
    goto EXIT;
  };
  sprintf (cmd, "%s %s  %s -o %s/exec", matching_cmd->cmd, matching_cmd->args, compile_file, tmpdir);
  if ((errno = system (cmd), errno != 0)) {
    fprintf(stderr, "error: compiler failed with code %d\n", errno);
    goto EXIT;
  }

  sprintf (excmd, "%s/exec", tmpdir);
  if ((errno = system (excmd), errno != 0))
    fprintf (stderr, "error: binary failed with code %d\n", errno);


 EXIT:
  for (int i = 0; i < ccmds_len; i++)
    compile_cmd_free (ccmds[i]);
  if (excmd) free (excmd);
  if (ccmds) free (ccmds);
  if (tmpdir) free (tmpdir);
  if (cmd) free (cmd);
  if (compile_file) free (compile_file);
  exit(errno % 255);
}
