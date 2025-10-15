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
#include "iniparser.h"
#include "io/file.h"

/*
The grammar of an INI file how it is expected by this module is as follows:


 


*/

typedef struct {
    Table *table;
    const char *text;
    char *current;
} IniParser;

static void IniParser_Init(IniParser *parser) {
    parser->table = Table_Create();
    parser->text = NULL;
    parser->current = NULL;
}

static void IniParser_Deinit(IniParser *parser) {
    Table_Destroy(parser->table);
    parser->table = NULL;
    parser->text = NULL;
    parser->current = NULL;
}

static bool is_any_of(char ch, const char *accept) {
    for ( ;*accept; accept++) {
        if (ch == *accept) {
            return true;
        }
    }
    return false;
}

static bool is_any_except(char ch, const char *except) {
    for ( ;*except; except++) {
        if (ch == *except) {
            return false;
        }
    }
    return true;
}

static bool is_whitespace(char ch) {
    return is_any_of(ch, " \t");
}

static bool is_char(char ch) {
    return is_any_except(ch, "\n\r\"\\");
}

static bool is_barechar(char ch) {
    return is_any_except(ch, "\"#;\n\r\t");
}
