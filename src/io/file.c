#include "file.h"

#include <stdio.h>
#include <stdlib.h>
#include "common/logging.h"

static File *create_file_object() {
    File *file = malloc(sizeof(File));
    if (!file) {
        logFatal("Cannot allocate memory for File.");
    }
    file->path = NULL;
    return file;
}

File *File_Open(const char *filename) {
    File *file = create_file_object();
    file->path = filename;

    FILE *fp = fopen(filename, "r");
    if (!fp) {
        logFatal("Cannot open file %s.", filename);
    }

    return file;
}

void File_Close(File *file) {
    fclose(file->fp);
    free(file);
}

UTF8String *File_ReadLine(File *file) {
    UTF8String *line = UTF8String_Create();
    char *lineptr = NULL;
    size_t length;
    getline(&lineptr, &length, file->fp);
    if (lineptr[length-1] == '\n') {
        lineptr[length-1] = '\0';
        length--;
    }
    UTF8String_FromString(line, lineptr);
    free(lineptr);
    return line;
}

void File_WriteLine(File *file, const UTF8String *line) {
    char *str = UTF8String_ToStr(line);
    fputs(str, file->fp);
    free(str);
}
