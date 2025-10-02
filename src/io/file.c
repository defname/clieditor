#include "file.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
        logFatal("Cannot open file %s.", filename);
    }
    file->access = access;

    return file;
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

UTF8String *File_ReadLine(File *file) {
    if (!file || !file->fp || file->access != FILE_ACCESS_READ) {
        logError("Invalid file handle.");
        return NULL;
    }
    UTF8String *line = UTF8String_Create();
    char *lineptr = NULL;
    size_t length = 0;
    ssize_t bytes_read = getline(&lineptr, &length, file->fp);
    if (bytes_read == -1) {
        free(lineptr);
        UTF8String_Destroy(line);
        return NULL;
    }
    if (bytes_read > 0 && lineptr[bytes_read-1] == '\n') {
        lineptr[--bytes_read] = '\0';
    }
    if (bytes_read > 0 && lineptr[bytes_read-1] == '\r') {
        lineptr[--bytes_read] = '\0';
    }
    UTF8String_FromStr(line, lineptr, (size_t)bytes_read);
    free(lineptr);
    return line;
}

void File_WriteLine(File *file, const UTF8String *line) {
    if (!file || !file->fp || !line || file->access != FILE_ACCESS_WRITE) {
        logError("Invalid parameters for File_WriteLine.");
        return;
    }
    char *str = UTF8String_ToStr(line);
    fputs(str, file->fp);
    free(str);
    fputc('\n', file->fp);
}
