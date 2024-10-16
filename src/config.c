/* config.c
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

#define _XOPEN_SOURCE 500
#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE
#include <stdlib.h>
#include <extlib.h>
#include <ftw.h>
#include <stdio.h>
#include <string.h>
#include "config.h"

struct compile_cmd_t ***ccmds;
size_t ccmds_size = 0;

struct compile_cmd_t *
compile_cmd_init ()
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
    free (self->cmd);
    free (self->args);
    free (self->fileext);
    free (self);
}

int
parse_config (const char *fpath, const struct stat *sb,
              int typeflag, struct FTW *ftwbuf)
{
    if (!S_ISREG (sb->st_mode))
        return 0;
    if (sb->st_size < 21)       // config file that doesnt include "compiler=x" and "fileext=x" can be ignored
        return 0;

    struct compile_cmd_t *ccmd = compile_cmd_init ();
    struct compile_cmd_t **ccmds_tmp =
        realloc (*ccmds, sizeof (ccmds) + sizeof (struct compile_cmd_t));

    if (ccmds_tmp)
        *ccmds = ccmds_tmp;

    FILE *cfg = fopen (fpath, "r");

    int CMD_SET = 1;
    int FILEEXT_SET = 2;

    int filled = 0;
    while (filled != (CMD_SET | FILEEXT_SET)) {
        char *line = NULL;
        size_t lineBufSize = 0;
        getline (&line, &lineBufSize, cfg);
        char *identifier = strsep (&line, "=");
        if (!line)
            fprintf (stderr, "warn: invalid configuration: %s\n", fpath);
        else {
            if (strcmp (identifier, "compiler") == 0 && !(filled & CMD_SET)) {
                ccmd->cmd = trim (line, NULL, NULL);
                filled = filled | CMD_SET;
            }
	    else if (strcmp (identifier, "args") == 0) {
                free (ccmd->args);
                ccmd->args = trim (line, NULL, NULL);
            }
            else if (strcmp (identifier, "fileext") == 0
                     && !(filled & FILEEXT_SET)) {
                ccmd->fileext = trim (line, NULL, NULL);
                filled = filled | FILEEXT_SET;
            }
            else {
                fprintf (stderr, "warn: invalid configuration: %s\n", fpath);
                free (identifier);
                free (line);
                fclose (cfg);
                free (ccmd);
                return 1;
            }
        }
	free (identifier);
    }
    fclose (cfg);
    ccmds_tmp[ccmds_size] = ccmd;
    ccmds_size += 1;
    return 0;
}

size_t
get_commands (struct compile_cmd_t ***ccmds_glob)
{
    ccmds = ccmds_glob;
    *ccmds = malloc (sizeof (struct compile_cmd_t));
    char *home = getenv ("HOME");
    if (!home) {
        fprintf (stderr,
                 "warn: HOME not found. Unable to read home configuration files.\n");
    }
    char *paths[2] = { "/etc/runc.d", NULL };
    if (home) {
        paths[1] = malloc (strlen (home) + strlen ("/.config/runc") + 1);
        sprintf (paths[1], "%s/.config/runc", home);
    }
    for (int i = 0; i < 2; i++) {
        if (!paths[i])
            break;
        nftw (paths[i], parse_config, 10, FTW_DEPTH | FTW_MOUNT | FTW_PHYS);
        if (i == 1)
            free (paths[i]);
    }
    return ccmds_size;
}
