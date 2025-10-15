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

/**
 * iniparser.h
 * 
 * Provides a easy way to read INI files into hash Tables.
 */

/********** INI-File Grammar *************

<INIFILE>       ::= (<WS>* <EXPR>)* <WS>*

<EXPR>          ::= <COMMENT>
                  | <SECTION>
                  | <ASSIGNMENT>
                  | <EMPTYLINE>)

<SECTION>       ::= '[' <WS>* <KEY> <WS>* ']' <WS>* <OPT_COMMENT>? '\n' <ASSIGNMENT>* 

<ASSIGNMENT>    ::= <KEY> <WS>* '=' <WS>* <VALUE>? <WS>* <OPT_COMMENT>? '\n'

<VALUE>         ::= <NUMBER>
                  | <STRING>
                  | <BARESTRING>

<NUMBER>        ::= <DIGIT>+

<STRING>        ::= '"' <CHAR>* '"'
<CHAR>          ::= [^\n\r"\\] | <ESCAPE>
<ESCAPE>        ::= '\' ['"nrt\\]

<BARESTRING>    ::= <BARECHAR>+
<BARECHAR>      ::= [^"#;\n\r\t]

<COMMENT>       ::= <WS>* (';' | '#') <TEXT>* '\n'
<OPT_COMMENT>   ::= <WS>* (';' | '#') <TEXT>*

<TEXT>          ::= [^\n]*
<DIGIT>         ::= [0-9]

<EMPTYLINE>     ::= <WS>* '\n'

<WS>            ::= ' ' | '\t'

<LETTER>        ::= [a-zA-Z]
<KEY_CHAR>      ::= <LETTER> | <DIGIT> | '.' | '-' | '_'
<KEY>           ::= <LETTER> <KEY_CHAR>*

*****************************************/
#ifndef INIPARSER_H
#define INIPARSER_H

#include "table.h"

typedef struct _ParsingError {
    int line;
    int column;
    char *message;
} ParsingError;

ParsingError *ParsingError_Create(int line, int column, const char *msg);
void ParsingError_Destroy(ParsingError *error);


typedef struct {
    Table *table;
    Table *current_section;

    const char *text;
    const char *current;
    int line;
    int column;
    ParsingError *error;
} IniParser;

void IniParser_Init(IniParser *parser);
void IniParser_Deinit(IniParser *parser);
void IniParser_Reset(IniParser *parser);

Table *IniParser_Parse(IniParser *parser);

#endif