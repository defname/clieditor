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
#ifndef FILE_H
#define FILE_H

#include "stdio.h"
#include "common/utf8string.h"

typedef enum {
    FILE_ACCESS_READ,
    FILE_ACCESS_WRITE
} FileAccessType;

/**
 * @brief File structure to hold all necessary information about a open file.
 */
typedef struct _File {
    char *path;             //< pathname of the file
    FILE *fp;               //< file handler
    FileAccessType access;  //< read/write access
} File;

/**
 * @brief Open a file for read/write access.
 * 
 * This function *creates* a new File instance, that needs to be closed to free it.
 * @param path filepath
 * @returns the created File instance
 */
File *File_Open(const char *path, FileAccessType access);

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
 *          UTF8String_Destroy()
 */
UTF8String *File_ReadLine(File *file);

/**
 * @brief Read whole file. Ownership is left to caller
 */
char *File_Read(File *file);

/**
 * @brief Write a line to a file.
 */
void File_WriteLine(File *file, const UTF8String *line);

#endif