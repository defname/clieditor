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
 * table.h
 * 
 * Simple implementation of a hashtable using the open addressing
 * with linear probing. For hashing the djb2 hash algorithm is used.
 */
#ifndef TABLE_H
#define TABLE_H

#include <stdlib.h>

#define TABLE_INITIAL_CAPACITY 16
#define TABLE_GROWTH_FACTOR 2
#define TABLE_MAX_LOAD_FACTOR 0.75


struct _Table;

typedef union _TableEntryValue {
    int int_value;
    char *string_value;
    struct _Table *table_value;
} TableValue;

typedef enum {
    TABLE_VALUE_TYPE_NONE,
    TABLE_VALUE_TYPE_INT,
    TABLE_VALUE_TYPE_STRING,
    TABLE_VALUE_TYPE_TABLE
} TableValueType;

typedef enum {
    TABLE_SLOT_EMPTY,
    TABLE_SLOT_USED,
    TABLE_SLOT_TOMBSTONE
} TableSlotState;

typedef struct _TableEntry {
    const char *key;
    TableValue value;
    TableValueType type;
    TableSlotState state;
} TableSlot;

void TableEntry_Init(TableSlot *slot);
void TableEntry_Deinit(TableSlot *slot);

typedef struct _Table {
    TableSlot *slots;
    size_t capacity;
    size_t used;
} Table;

void Table_Init(Table *table);
void Table_Deinit(Table *table);

Table *Table_Create();
void Table_Destroy(Table *table);

/**
 * @brief Set value for key in table.
 * 
 * If the key does not already exist in table it is copied with strdup(). The data a pointer in value
 * points to is not copied, but Table takes the ownership of this pointers and will free them if the
 * value gets overwritten or the table gets freed.
 */
void Table_Set(Table *table, const char *key, TableValueType type, TableValue value);

/**
 * @brief Find the value for key in the table and return it.
 * 
 * If key exists in the table return the value type and set value to the found value.
 * If the key does not exist return TABLE_VALUE_TYPE_NONE and set value to fallback.
 */
TableValueType Table_Get(Table *table, const char *key, TableValue *value, TableValue fallback);

/**
 * @brief Delete the entry for key in table.
 * 
 * Delete the entry of key if it exists. If value contains a pointer it is freed properly.
 * (for Table* Table_Destroy() is called)
 */
void Table_Delete(Table *table, const char *key);


void Table_SetInt(Table *table, const char *key, int value);
void Table_SetStr(Table *table, const char *key, const char *value);
void Table_SetTable(Table *table, const char *key, Table *value);



#endif