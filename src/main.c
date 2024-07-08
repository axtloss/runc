#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#define _XOPEN_SOURCE 500
#include <extlib.h>
#include "config.h"

int main (int argc, char *argv[]) {
  char *compiler = "gcc";
  char *tmplate = strdup ("/tmp/XXXXXX");
  if (!tmplate)
    exit (1);
  char *tmpdir = mkdtemp(tmplate);
  if (!tmpdir)
    exit (1);
  char *cmd = malloc (strlen (compiler)+strlen(argv[argc-1])+strlen ("  -o  ")+strlen (tmpdir)+strlen ("/meow")+1);
  char *excmd = malloc (strlen (tmplate)+strlen ("/meow")+1);
  if (!cmd)
    exit (1);
  if (!excmd)
    exit (1);
  puts (tmpdir);
  sprintf (cmd, "%s %s -o %s/meow", compiler, argv[argc-1], tmpdir);
  system (cmd);
  sprintf (excmd, "%s/meow", tmpdir);
  system (excmd);
  int rtrn = rrmdir (tmpdir);
  if (rtrn != 0)
    printf ("%d %d\n", rtrn, errno);
  get_commands();
  free (tmpdir);
  free (cmd);
  free (excmd);
  return 0;
}
