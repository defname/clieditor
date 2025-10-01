#ifndef FILE_H
#define FILE_H

#include "stdio.h"
#include "common/utf8string.h"

/**
 * @brief File structure to hold all necessary information about a open file.
 */
typedef struct _File {
    const char *path;       //< pathname of the file
    FILE *fp;               //< file handler
} File;

/**
 * @brief Open a file for read/write access.
 * 
 * This function *creates* a new File instance, that needs to be closed to free it.
 * @param path filepath
 * @returns the created File instance
 */
File *File_Open(const char *path);

/**
 * @brief Close a file
 * 
 * Close the file handler and free memory.
 * @param file pointer to the File instance
 */
void File_Close(File *file);

/**
 * @brief Read a line from a file.
 * 
 * @returns The read line as a new created UTF8String.
 *          The caller is responsible for freeing it with
 *          UTF8String_Free()
 */
UTF8String *File_ReadLine(File *file);

/**
 * @brief Write a line to a file.
 */
void File_WriteLine(File *file, const UTF8String *line);

#endif