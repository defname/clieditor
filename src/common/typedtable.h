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
 * typedtable.h
 * 
 * Provides functions which operate on a normal Table to use typed values.
 * Ownership is always taken (except when using TypedTable_SetStringCopy()).
 */
#ifndef TYPEDTABLE_H
#define TYPEDTABLE_H

#include "table.h"

#include <stdbool.h>

typedef enum {
    VALUE_TYPE_NUMBER,
    VALUE_TYPE_STRING,
    VALUE_TYPE_BOOLEAN,
    VALUE_TYPE_TABLE,
    VALUE_TYPE_NONE
} ValueType;

typedef struct _TypedValue {
    ValueType type;
    union {
        int number_value;
        char* string_value;
        int boolean_value;
        Table *table_value;
    } data;
} TypedValue;

TypedValue *TypedValue_Create();
void TypedValue_Destroy(TypedValue *value);

void TypedTable_SetNumber(Table *table, const char *key, int value);
void TypedTable_SetString(Table *table, const char *key, char *value);
void TypedTable_SetStringCopy(Table *table, const char *key, const char *value);
void TypedTable_SetBoolean(Table *table, const char *key, bool value);
void TypedTable_SetTable(Table *table, const char *key, Table *value);

ValueType TypedTable_GetType(Table *table, const char *key);

int TypedTable_GetNumber(Table *table, const char *key);
const char* TypedTable_GetString(Table *table, const char *key);
bool TypedTable_GetBoolean(Table *table, const char *key);
Table* TypedTable_GetTable(Table *table, const char *key);


#endif