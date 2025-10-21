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
 * 
 * The load factor is calculated from all slots that are not empty
 * (tombstones included). It's assumed that there not too many deletions
 * occure. Otherwise the worst that can happen is that the used memory
 * increases. That's the tradeoff for fast access through less probing.
 */
#ifndef TABLE_H
#define TABLE_H

#include <stdlib.h>
#include <stdbool.h>

#define TABLE_INITIAL_CAPACITY 16
#define TABLE_GROWTH_FACTOR 2
#define TABLE_MAX_LOAD_FACTOR 0.75

typedef enum {
    TABLE_SLOT_EMPTY,
    TABLE_SLOT_USED,
    TABLE_SLOT_TOMBSTONE
} TableSlotState;

typedef struct _TableSlot {
    char *key;
    void *value;
    void (*destructor)(void *value);
    TableSlotState state;
} TableSlot;

void TableSlot_Init(TableSlot *slot);
void TableSlot_Deinit(TableSlot *slot);

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
 * If destructor is not NULL table takes the ownership of of value and will take responsibility
 * for destroying it (in the case of overwriting or deleting it). If destructor is NULL the
 * ownership stays by the owner and he need to take care of freeing value.
 */
void Table_Set(Table *table, const char *key, void *value, void (*destructor)(void *value));

/**
 * @brief Return the value for key or NULL if key is not found.
 * 
 * If NULL is a ligit value for key use Table_Has() to check if the key exists in the table.
 */
void *Table_Get(const Table *table, const char *key);

/**
 * @brief Delete the entry for key in table.
 * 
 * Delete the entry of key if it exists. If table has the ownership for value (a destructor is present)
 * value will be destructed. 
 */
void Table_Delete(Table *table, const char *key);

/**
 * @brief Return true if the key exists in table.
 */
bool Table_Has(const Table *table, const char *key);

/**
 * @brief Return true if table has the ownership for the entry of key
 */
bool Table_HasOwnership(const Table *table, const char *key);

/**
 * @brief Return the current number of non-free table slots.
 */
size_t Table_GetUsage(const Table *table);

#endif