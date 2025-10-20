#ifndef SYNTAX_DEFINITION_H
#define SYNTAX_DEFINITION_H

#include <stdint.h>
#include <regex.h>
#include "common/table.h"
#include "common/string.h"

/**
 * @brief Holds the definition of a syntax block.
 */
typedef struct _SyntaxBlockDef {
    char name[12];      //< name of the block definition. for easier debugging
    regex_t start;      //< compiled regex to determine the begin of the block
    regex_t end;        //< compiled regex to determine the end of the block (optional)
    
    bool only_start;    //< if true only start is tested and no children are allowed
    
    struct _SyntaxBlockDef **children;  //< list of children definition which are allowed inside the block
    size_t children_count;              //< number of children
    
    uint8_t color;      //< the color to render the block
} SyntaxBlockDef;

SyntaxBlockDef *SyntaxBlockDef_Create();
void SyntaxBlockDef_Destroy(SyntaxBlockDef *block);


/**
 * @brief Holds a complete syntax definition.
 */
typedef struct _SyntaxDefinition {
    char *name;                 //< name of the filetype (e.g. INI)
    SyntaxBlockDef *root;       //< root block definition
    SyntaxBlockDef **blocks;    //< list of all blocks
} SyntaxDefinition;

/**
 * @brief Constructs a SyntaxDefinition from a typed Table.
 * 
 * @param table A typed Table (see common/typedtable.h) holding the syntax definition.
 * 
 * @returns
 * A SyntaxDefinition or NULL if the definition in table is not valid.
 */
SyntaxDefinition *SyntaxDefinition_FromTable(const Table *table);
void SyntaxDefinition_Destroy(SyntaxDefinition *def);


#endif