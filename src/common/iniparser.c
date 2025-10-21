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
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "common/logging.h"
#include "common/typedtable.h"


ParsingError *ParsingError_Create(int line, int column, const char *msg) {
    ParsingError *error = malloc(sizeof(ParsingError));
    if (!error) {
        logFatal("Cannot allocate memory for ParsingError.");
    }
    error->line = line;
    error->column = column;
    error->message = strdup(msg);
    if (!error->message) {
        logFatal("Cannot allocate memory for error message.");
    }
    return error;
}

void ParsingError_Destroy(ParsingError *error) {
    if (!error) {
        return;
    }
    if (error->message) {
        free(error->message);
    }
    free(error);
}

/*********************************************/
/* String helper functions                   */

// create a new string from a substring 
static char *substr(const char *str, size_t length) {
    char *out = malloc(length + 1);
    if (!out) {
        logFatal("Cannot allocate memory in substr().");
    }
    memcpy(out, str, length);
    out[length] = '\0';
    return out;
}

// unescape string
char *unescape(char  *str) {
    size_t idx = 0;
    size_t new_idx = 0;
    for (;str[idx]; idx++, new_idx++) {
        if (str[idx] == '\\') {
            if (str[idx + 1] == '\0') {
                break;
            }
            switch (str[idx + 1]) {
                case 'n':
                    str[new_idx] = '\n';
                    break;
                case 't':
                    str[new_idx] = '\t';
                    break;
                case '"':
                    str[new_idx] = '"';
                    break;
                case '\\':
                    str[new_idx] = '\\';
                    break;
                default:
                    str[new_idx++] = '\\';
                    str[new_idx] = str[idx + 1];
                    logWarn("Invalid escape sequence in unescape()");
                    break;
            }
            idx++;
        }
        else {
            str[new_idx] = str[idx];
        }
    }
    str[new_idx] = '\0';
    return str;
}

/*************************************/
/* Character Set Helper functions    */
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

static bool is_newline(char ch) {
    return is_any_of(ch, "\n\r");
}

static bool is_char(char ch) {
    return is_any_except(ch, "\n\r\"\\");
}

static bool is_barechar(char ch) {
    return is_any_except(ch, "\"#;\n\r\t");
}

static bool is_letter(char ch) {
    return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z');
}

static bool is_digit(char ch) {
    return (ch >= '0' && ch <= '9');
}

static bool is_key_char(char ch) {
    return is_letter(ch) || is_digit(ch) || ch == '.' || ch == '-' || ch == '_' || ch == ':';
}

static bool is_at_end(const IniParser *parser) {
    return *parser->current == '\0';
}

/***************************************/
/* Parser helper functions             */
static char peek(const IniParser *parser) {
    return *parser->current;
}

static char advance(IniParser *parser) {
    if (is_at_end(parser)) return '\0';

    if (peek(parser) == '\n') {
        parser->line++;
        parser->column = 0;
    } else {
        parser->column++;
    }
    parser->current++;
    return parser->current[-1];
}

static void set_error(IniParser *parser, const char *message) {
    if (parser->error) {
        return;
    }
    parser->error = ParsingError_Create(parser->line, parser->column, message);
}

static void skip_whitespace(IniParser *parser) {
    while (is_whitespace(peek(parser)) && !is_at_end(parser)) {
        advance(parser);
    }
}

static void expect_newline(IniParser *parser) {
    if (!is_newline(peek(parser)) && !is_at_end(parser)) {
        set_error(parser, "Expected newline.");
        return;
    }
    if (!is_at_end(parser)) {
        advance(parser);
    }
}


/***************************************/
/* Parsing functions                   */
static char *parse_key(IniParser *parser) {
    const char *key = parser->current;
    size_t length = 0;
    while (is_key_char(peek(parser)) && !is_at_end(parser)) {
        advance(parser);
        length++;
    }
    return substr(key, length);
}

static void parse_text(IniParser *parser) {
    while (!is_newline(peek(parser)) && !is_at_end(parser)) {
        advance(parser);
    }
}

static void parse_opt_comment(IniParser *parser) {
    skip_whitespace(parser);
    if (is_any_of(peek(parser), "#;")) {
        advance(parser);
        parse_text(parser);
    }
}

static char *parse_barestring(IniParser *parser) {
    const char *bare_str = parser->current;
    size_t length = 0;
    while (is_barechar(peek(parser)) && !is_at_end(parser)) {
        advance(parser);
        length++;
    }
    // strip whitespaces from end
    while (length > 0 && is_whitespace(bare_str[length - 1])) {
        length--;
    }
    return substr(bare_str, length);
}

static char *parse_string(IniParser *parser) {
    const char *str = parser->current;
    size_t length = 0;
    while ((is_char(peek(parser)) || (peek(parser) == '\\')) && !is_at_end(parser)) {
        if (peek(parser) == '\\') {
            advance(parser);
            length++;
            if (!is_any_of(peek(parser), "\"nt\\")) {
                set_error(parser, "Expected a valid escape sequence.");
                return NULL;
            }
        }
        advance(parser);
        length++;
    }
    return unescape(substr(str, length));
}

static char *parse_number(IniParser *parser) {
    const char *number = parser->current;
    size_t length = 0;
    while (is_digit(peek(parser)) && !is_at_end(parser)) {
        advance(parser);
        length++;
    }
    return substr(number, length);
}

static void parse_value(IniParser *parser, const char *key) {
    if (peek(parser) == '"') {
        advance(parser);
        char *value = parse_string(parser);
        if (!value) {
            return;  // error already set
        }
        if (peek(parser) != '"') {
            set_error(parser, "Expected '\"'");
            free(value);
            return;
        }
        advance(parser);
        TypedTable_SetString(parser->current_section, key, value);
    }
    // need to check for number value before barestring before a barestring can contain numbers
    else if (is_digit(peek(parser))) {
        char *value = parse_number(parser);
        int number = atoi(value);
        free(value);
        TypedTable_SetNumber(parser->current_section, key, number);
    }
    else if (is_barechar(peek(parser))) {
        char *value = parse_barestring(parser);
        TypedTable_SetString(parser->current_section, key, value);
    }
    else {
        TypedTable_SetStringCopy(parser->current_section, key, "");
    }
}

static void parse_assignment(IniParser *parser) {
    char *key = parse_key(parser);
    skip_whitespace(parser);
    if (peek(parser) != '=') {
        set_error(parser, "Expected '='.");
        free(key);
        return;
    }
    advance(parser);
    skip_whitespace(parser);
    parse_value(parser, key);
    free(key);
    parse_opt_comment(parser);
    expect_newline(parser);
}

static void parse_section(IniParser *parser) {
    advance(parser);
    skip_whitespace(parser);
    if (!is_key_char(peek(parser))) {
        set_error(parser, "Expected key.");
        return;
    }
    char *key = parse_key(parser);
    skip_whitespace(parser);
    if (peek(parser) != ']') {
        set_error(parser, "Expected ']'");
        free(key);
        return;
    }
    advance(parser);
    parser->current_section = Table_Create();
    TypedTable_SetTable(parser->table, key, parser->current_section);
    free(key);
    parse_opt_comment(parser);
    expect_newline(parser);
}

static void parse_expr(IniParser *parser) {
    if (peek(parser) == '[') {
        parse_section(parser);
    }
    else if (is_key_char(peek(parser))) {
        parse_assignment(parser);
    }
    else if (is_newline(peek(parser))) {
        advance(parser);
    }
    else if (is_any_of(peek(parser), "#;")) {
       parse_text(parser);
       expect_newline(parser);
    }
    else {
        set_error(parser, "Unexpected character.");
    }
}

static void parse_inifile(IniParser *parser) {
    while (!is_at_end(parser)) {
        if (parser->error) {
            break;
        }
        skip_whitespace(parser);
        parse_expr(parser);
    }
}


void IniParser_Init(IniParser *parser) {
    parser->text = NULL;
    parser->table = NULL;
    parser->error = NULL;
    IniParser_Reset(parser);
}

void IniParser_Deinit(IniParser *parser) {
    IniParser_Reset(parser);
    parser->text = NULL;
}

void IniParser_Reset(IniParser *parser) {
    if (parser->error) {
        ParsingError_Destroy(parser->error);
    }
    if (parser->table) {
        Table_Destroy(parser->table);
    }
    parser->error = NULL;
    parser->table = NULL;
    parser->current_section = NULL;
    parser->current = parser->text;
    parser->line = 1;
    parser->column = 0;
}

void IniParser_SetText(IniParser *parser, const char *text) {
    parser->text = text;
}

const char *IniParser_GetText(const IniParser *parser) {
    return parser->text;
}

const ParsingError *IniParser_GetError(const IniParser *parser) {
    return parser->error;
}

Table *IniParser_Parse(IniParser *parser) {
    IniParser_Reset(parser);
    if (!parser->text) {
        set_error(parser, "No text to parse.");
        return NULL;
    }
    parser->table = Table_Create();
    parser->current_section = parser->table;
    parse_inifile(parser);

    if (parser->error) {
        Table_Destroy(parser->table);
        parser->table = NULL;
        return NULL;
    }

    // Success: Transfer ownership to the caller
    Table *result_table = parser->table;
    parser->table = NULL;
    return result_table;
}
