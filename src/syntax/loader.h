#ifndef SYNTAX_LOADER_H
#define SYNTAX_LOADER_H

#include "highlighting.h"
#include "common/iniparser.h"

typedef enum {
    SYNTAX_LOADER_NO_ERROR,
    SYNTAX_LOADER_NO_FILENAME,
    SYNTAX_LOADER_FILE_NOT_FOUND,
    SYNTAX_LOADER_FILE_READ_ERROR,
    SYNTAX_LOADER_PARSE_ERROR,
    SYNTAX_LOADER_DEFINITION_ERROR
} SyntaxHighlightingLoaderErrorCode;

typedef struct {
    SyntaxHighlightingLoaderErrorCode code;
    SyntaxDefinitionError def_error;
    ParsingError parsing_error;
} SyntaxHighlightingLoaderError;

void SyntaxHighlightingLoaderError_Deinit(SyntaxHighlightingLoaderError *error);


/**
 * @brief Load a syntax definition and create the highlight engine.
 * 
 * Load the syntax definition for `format` from data/sytnax/<format>.ini
 * (from inside a project's folder) and create the `SyntaxHighlight` instance.
 * 
 * @param format Format to load (something like "ini", "md").
 * @param error Will be set in the case of an error and need to be deinitialized by the caller.
 * @returns
 * The newly created SyntaxHighlighting Instance (ownership transfers to the caller).
 * NULL there is definition file or if an error occured.
 * In the case of an error `error` will be set and need to be deinitialized by the caller. 
 * 
 * Note theat `format` needs to be exactly the same as the filename.
 * The convention is to use lower case.
 */
SyntaxHighlighting *SyntaxHighlighting_LoadFromFile(const char *format, SyntaxHighlightingLoaderError *error);

#endif