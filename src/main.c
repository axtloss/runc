/* main.c
 *
 * Copyright 2024 axtlos <axtlos@disroot.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define _XOPEN_SOURCE 500
#include <extlib.h>
#include "config.h"

struct compile_cmd_t *
find_cmd (char *file_extension, struct compile_cmd_t **ccmds,
          size_t ccmds_len)
{
    for (int i = 0; i < ccmds_len; i++) {
        if (strcmp (ccmds[i]->fileext, file_extension) == 0) {
            return ccmds[i];
        }
    }
    return NULL;
}

void
remove_shebang (char **file, char *tmpdir, char *file_extension)
{
    char *new_file_path = malloc (strlen (tmpdir) + strlen ("/source.") +
				  strlen (file_extension) + 1);
    sprintf (new_file_path, "%s/source.%s", tmpdir, file_extension);
    FILE *new_file = fopen (new_file_path, "w");
    FILE *old_file = fopen (*file, "r");
    char buffer[1024];
    fgets (buffer, 1024, old_file);
    fcopy (old_file, new_file);
    fclose (new_file);
    fclose (old_file);
    free (*file);
    *file = strdup (new_file_path);
    free (new_file_path);
}

int
main (int argc, char *argv[])
{
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
    char *fileparts[strlen (argv[argc - 1])];
    char *extra_args = NULL;

    if (argc < 2) {
        fprintf (stderr, "usage: %s <source file>", argv[0]);
        errno = 1;
        goto EXIT;
    }
    else if (argc > 2) {
	extra_args = join_str(argv, argc-1, " ");
    }
    compile_file = strdup (argv[argc - 1]);
    ccmds_len = get_commands (&ccmds);

    tmpdir = mkdtemp (tmpdir);
    if (!tmpdir) {
        errno = 1;
        goto EXIT;
    }

    for (int i = 0; (ap = strsep (&argv[argc - 1], ".")); i++) {
        fileparts[i] = ap;
        parts_len += 1;
    }
    if (parts_len == 0) {
        fprintf (stderr, "error: no file extension found\n");
        errno = 1;
        goto EXIT;
    }

    matching_cmd = find_cmd (fileparts[parts_len - 1], ccmds, ccmds_len);
    if (!matching_cmd) {
        fprintf (stderr, "error: no complier for file extension '%s' found\n",
                 fileparts[parts_len - 1]);
        errno = 1;
        goto EXIT;
    }

    FILE *source = fopen (compile_file, "r");
    char is_shebang[3];
    fread (&is_shebang, 1, 2, source);
    fclose (source);
    if (strcmp (is_shebang, "#!") == 0)
        remove_shebang (&compile_file, tmpdir, fileparts[parts_len - 1]);

    cmd = malloc (strlen (matching_cmd->cmd) +
                  strlen (matching_cmd->args) +
                  strlen (compile_file) +
		  strlen (extra_args == NULL ? "" : extra_args + strlen (argv[0]) + 1) +
                  strlen ("   -o   ") +
                  strlen (tmpdir) + strlen ("/exec") + 1);
    if (!cmd) {
        errno = 1;
        goto EXIT;
    }
    excmd = malloc (strlen (tmpdir) + strlen ("/exec") + 1);
    if (!excmd) {
        errno = 1;
        goto EXIT;
    }
    sprintf (cmd, "%s %s %s %s -o %s/exec", matching_cmd->cmd,
             matching_cmd->args, extra_args == NULL ? "" : extra_args + strlen (argv[0]) + 1, compile_file, tmpdir);
    if ((errno = system (cmd), errno != 0)) {
        fprintf (stderr, "error: compiler failed with code %d\n", errno);
        goto EXIT;
    }

    sprintf (excmd, "%s/exec", tmpdir);
    if ((errno = system (excmd), errno != 0)) {
        fprintf (stderr, "error: binary failed with code %d\n", errno);
        goto EXIT;
    }

    if ((errno = rrmdir (tmpdir)), errno != 0)
        fprintf (stderr, "error: failed to remove temporary directory %s\n",
                 tmpdir);

  EXIT:
    for (int i = 0; i < ccmds_len; i++)
        compile_cmd_free (ccmds[i]);
    free (excmd);
    free (ccmds);
    free (tmpdir);
    free (cmd);
    free (compile_file);
    free (extra_args);
    exit (errno % 255);
}
