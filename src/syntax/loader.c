#include "loader.h"
#include <string.h>
#include <stdio.h>  // snprintf
#include "io/file.h"

void SyntaxHighlightingLoaderError_Init(SyntaxHighlightingLoaderError *error) {
    memset(error, 0, sizeof(SyntaxHighlightingLoaderError));
    error->code = SYNTAX_LOADER_NO_ERROR;
}


void SyntaxHighlightingLoaderError_Deinit(SyntaxHighlightingLoaderError *error) {
    SyntaxDefinitionError_Deinit(&error->def_error);
    if (error->parsing_error.message) {
        free(error->parsing_error.message);
    }
}


SyntaxHighlighting *SyntaxHighlighting_LoadFromFile(const char *format, SyntaxHighlightingLoaderError *error) {
    if (!error) {
        return NULL;
    }
    SyntaxHighlightingLoaderError_Init(error);

    if (!format) {
        error->code = SYNTAX_LOADER_NO_FILENAME;
        return NULL;
    }

    // 1. load the ini file
    char filename[PATH_MAX];
    snprintf(filename, PATH_MAX, "data/syntax/%s.ini", format);
    File *file = File_OpenProjectFile(filename, FILE_ACCESS_READ);
    if (!file) {
        error->code = SYNTAX_LOADER_FILE_NOT_FOUND;
        return NULL;
    }
    
    // 2. read the ini file
    char *ini = File_Read(file);
    if (!ini) {
        error->code = SYNTAX_LOADER_FILE_READ_ERROR;
        File_Close(file);
        return NULL;
    }
    File_Close(file);

    // 3. parse the ini file
    IniParser parser;
    IniParser_Init(&parser);
    IniParser_SetText(&parser, ini);
    Table *table = IniParser_Parse(&parser);
    if (!table) {
        error->code = SYNTAX_LOADER_PARSE_ERROR;
        error->parsing_error = *IniParser_GetError(&parser);
        error->parsing_error.message = strdup(IniParser_GetError(&parser)->message);
        IniParser_Deinit(&parser);
        free(ini);
        return NULL;
    }
    IniParser_Deinit(&parser);
    free(ini);

    // 4. create the syntax definition
    SyntaxDefinition *def = SyntaxDefinition_FromTable(table, &error->def_error);
    if (!def) {
        Table_Destroy(table);
        error->code = SYNTAX_LOADER_DEFINITION_ERROR;
        return NULL;
    }
    Table_Destroy(table);

    // 5. create the highlighting engine
    return SyntaxHighlighting_Create(def);
}