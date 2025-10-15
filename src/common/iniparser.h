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

<INIFILE>       ::= <WS>* <EXPR>* <WS>*

<EXPR>          ::= <COMMENT>
                  | <SECTION>
                  | <ASSIGNMENT>
                  | <EMPTYLINE>

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

<KEY>           ::= <BARECHAR>+

<WS>            ::= (' ' | '\t')*

*****************************************/
#ifndef INIPARSER_H
#define INIPARSER_H

#include "table.h"

Table *IniParser_Parse(const char *ini);
Table *IniParser_ParseFile(const char *filename);

#endif