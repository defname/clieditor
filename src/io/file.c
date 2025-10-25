/* Copyright (C) 2025 defname
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "file.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>     // access()
#include <libgen.h>     // dirname()
#include <limits.h>     // PATH_MAX
#include <errno.h>
#include "common/config.h"
#include "common/logging.h"

static File *create_file_object() {
    File *file = malloc(sizeof(File));
    if (!file) {
        logFatal("Cannot allocate memory for File.");
    }
    file->path = NULL;
    file->fp   = NULL;
    file->access = FILE_ACCESS_READ;
    return file;
}

bool File_Exists(const char *path) {
    if (!path) {
        return false;
    }
    return (access(path, R_OK) == 0);
}

File *File_Open(const char *filename, FileAccessType access) {
    if (!filename || strlen(filename) == 0) {
        logFatal("Invalid filename.");
    }
    File *file = create_file_object();
    file->path = malloc(strlen(filename) + 1);
    if (!file->path) {
        logFatal("Cannot allocate memory for filename.");
    }
    strcpy(file->path, filename);

    file->fp = fopen(filename, access == FILE_ACCESS_READ ? "r" : "w");
    if (!file->fp) {
        free(file->path);
        free(file);
        return NULL;
    }
    file->access = access;

    return file;
}

// find project file 
char *find_project_file(const char *rel_path) {
    static char path[PATH_MAX];
    char exe_dir[PATH_MAX];
    char *home = getenv("HOME");

    // 1️. directory of the binary
    if (realpath(Config_GetExePath(), exe_dir)) {
        strcpy(exe_dir, dirname(exe_dir));
        snprintf(path, sizeof(path), "%s/%s", exe_dir, rel_path);
        if (File_Exists(path)) {
            return path;
        }
    }

    // 2️. user config (~/.config/clieditor/config)
    if (home) {
        snprintf(path, sizeof(path), "%s/.config/clieditor/%s", home, rel_path);
        if (File_Exists(path)) {
            return path;
        }
    }

    // 3️ system config (/usr/share/clieditor/config)
    const char *system_paths[] = {
        "/etc/clieditor/",
        "/usr/share/clieditor/",
        NULL
    };
    for (int i = 0; system_paths[i]; ++i) {
        snprintf(path, sizeof(path), "%s/%s", system_paths[i], rel_path);
        if (File_Exists(path)) {
            return path;
        }
    }

    // nothing found
    return NULL;
}

File *File_OpenConfig(FileAccessType access) {
    char *path = find_project_file("config.ini");
    if (!path) {
        return NULL;
    }
    return File_Open(path, access);
}

File *File_OpenProjectFile(const char *rel_path, FileAccessType access) {
    char *path = find_project_file(rel_path);
    if (!path) {
        return NULL;
    }
    return File_Open(path, access);
}

void File_Close(File *file) {
    if (!file) {
        return;
    }
    if (file->path) {
        free(file->path);
    }
    if (file->fp && file->fp != stdin) {
        fclose(file->fp);
    }
    free(file);
}

String *File_ReadLine(File *file) {
    if (!file || !file->fp || file->access != FILE_ACCESS_READ) {
        logError("Invalid file handle.");
        return NULL;
    }
    char *lineptr = NULL;
    size_t length = 0;
    ssize_t bytes_read = getline(&lineptr, &length, file->fp);
    if (bytes_read == -1) {
        free(lineptr);
        return NULL;
    }
    if (bytes_read > 0 && lineptr[bytes_read-1] == '\n') {
        lineptr[--bytes_read] = '\0';
    }
    if (bytes_read > 0 && lineptr[bytes_read-1] == '\r') {
        lineptr[--bytes_read] = '\0';
    }
    String *str = malloc(sizeof(String));
    if (!str) {
        logFatal("Cannot allocate memory for String.");
    }
    *str = String_TakeCStr(lineptr);
    return str;
}

char *File_Read(File *file) {
    if (!file || !file->fp || file->access != FILE_ACCESS_READ) {
        logError("Invalid file handle.");
        return NULL;
    }

    // filesize
    fseek(file->fp, 0, SEEK_END);
    long size = ftell(file->fp);
    rewind(file->fp);

    // allocate memory
    char *buffer = malloc(size + 1); // + "\0"
    if (!buffer) {
        return NULL;
    }

    // read file
    size_t read_bytes = fread(buffer, 1, size, file->fp);

    // add '\0' for termination
    buffer[read_bytes] = '\0';

    return buffer;
}

void File_WriteLine(File *file, const String *line) {
    if (!file || !file->fp || !line || file->access != FILE_ACCESS_WRITE) {
        logError("Invalid parameters for File_WriteLine.");
        return;
    }
    const char *str = String_AsCStr(line);
    fputs(str, file->fp);
    fputc('\n', file->fp);
    fflush(file->fp);
}
