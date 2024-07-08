#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
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
  struct compile_cmd_t **ccmds;
  size_t ccmds_len;

  ccmds_len = get_commands(&ccmds);

  size_t arglength = strlen (argv[argc-1]);
  char **ap, *fileparts[arglength], *inputstring;
  inputstring = strdup (argv[argc-1]);
  size_t parts_len = 0;
  for (ap = fileparts; (*ap = strsep (&inputstring, ".")) != NULL;) {
    if (**ap != '\0') {
      if (++ap >= &inputstring)
	break;
      parts_len += 1;
    }
  }

  struct compile_cmd_t *matching_cmd = find_cmd(fileparts[parts_len-1], ccmds, ccmds_len);
  if (!matching_cmd) {
    puts ("error: no complier for file extension found");
    return 1;
  }

  char *tmplate = strdup ("/tmp/XXXXXX");
  if (!tmplate)
    exit (1);
  char *tmpdir = mkdtemp(tmplate);
  if (!tmpdir)
    exit (1);
  char *cmd = malloc (strlen (matching_cmd->cmd)+strlen(matching_cmd->args)+strlen(argv[argc-1])+strlen ("   -o   ")+strlen (tmpdir)+strlen ("/exec")+1);
  char *excmd = malloc (strlen (tmplate)+strlen ("/exec")+1);
  if (!cmd)
    exit (1);
  if (!excmd)
    exit (1);
  puts (tmpdir);
  sprintf (cmd, "%s %s  %s -o %s/exec", matching_cmd->cmd, matching_cmd->args, argv[argc-1], tmpdir);
  int cmp_rtrn = system (cmd);
  if (cmp_rtrn != 0) {
    fprintf(stderr, "error: compiler failed");
    return 1;
  }
  sprintf (excmd, "%s/exec", tmpdir);
  puts (cmd);
  puts (excmd);
  int sys_rtrn = system (excmd);
  if (sys_rtrn != 0) {
    fprintf (stderr, "error: binary failed");
  }
  /*  int rtrn = rrmdir (tmpdir);
  if (rtrn != 0)
    printf ("%d %d\n", rtrn, errno);
  */
  for (int i = 0; i < ccmds_len; i++) {
    free (ccmds[i]->cmd);
    free (ccmds[i]);
  }
  free (ccmds);
  free (tmpdir);
  free (cmd);
  free (excmd);
  return 0;
}
