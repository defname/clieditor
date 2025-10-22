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
#include "table.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "logging.h"


void TableSlot_Init(TableSlot *slot) {
    slot->key = NULL;
    slot->value = NULL;
    slot->destructor = NULL;
    slot->state = TABLE_SLOT_EMPTY;
}

void TableSlot_Deinit(TableSlot *slot, void (*free_key_func)(void *key)) {
    if (!slot) {
        return;
    }
    if (slot->key && free_key_func) {
        free_key_func(slot->key);
    }
    if (slot->destructor) {
        slot->destructor(slot->value);
    }
   TableSlot_Init(slot);
}

// djb2
static uint32_t hash_string(const char *str) {
    uint32_t hash = 5381;
    unsigned char c;

    while ((c = (unsigned char)*str++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash;
}

static uint32_t hash_ptr(const void *p) {
    uintptr_t x = (uintptr_t)p;
    return (uint32_t)(x ^ (x >> 32));
}

static int cmp_ptr(const void *a, const void *b) {
    return a != b;
}

static void *cpy_ptr(const void *p) {
    return (void*)p;
}

static TableSlot *find_slot(const Table *table, const void *key) {
    if (!table || !key) {
        logFatal("Invalid table or key in find_slot()");
    }
    // the +1 is a hotfix for an edge case bug. It need to be ensured that this does not succeed table->capacity!
    if (table->used >= table->capacity * TABLE_MAX_LOAD_FACTOR + 1) {
        logFatal("Table too loaded.");  // this can only happen there is an error in the code
    }
    uint32_t hash = table->hash_func(key);
    size_t index = hash % table->capacity;
    TableSlot *tombstone = NULL;
    for (;;) {
        TableSlot *slot = &table->slots[index];
        if (slot->state == TABLE_SLOT_EMPTY) {
            if (tombstone != NULL) {
                return tombstone;
            }
            return slot;
        }
        else if (slot->state == TABLE_SLOT_TOMBSTONE && tombstone == NULL) {
            tombstone = slot;
        }
        else if (slot->state == TABLE_SLOT_USED && table->key_cmp_func(slot->key, key) == 0) {
            return slot;
        }

        index = (index + 1) % table->capacity;
        // since the load factor is tested in the beginning, there will always be a free slot
        // so it's ensured that the loop terminates
    }
    logFatal("That's not possible!");
}

static void shallow_copy_table(Table *dst, const Table *src) {
    if (!dst || !src) {
        return;
    }
    if (dst->capacity * TABLE_MAX_LOAD_FACTOR <= src->used) {
        // this prevents a infinit loop when TableSet is called.
        logFatal("Wrong use of copy_table. (dst->capacity too small)");
    }
    for (size_t i=0; i<src->capacity; i++) {
        TableSlot *src_slot = &src->slots[i];
        if (src_slot->state != TABLE_SLOT_USED) {
            continue;
        }
        // Table_Set() copies the key. The ownership of value is left by caller
        // or transfered to new table
        Table_Set(dst, src_slot->key, src_slot->value, src_slot->destructor);
        // the src table looses the ownership in any case
        src_slot->destructor = NULL;
    }
}

static void increase_capacity(Table *table) {
    if (!table) {
        return;
    }
    // allocate memory for the increased table
    Table *new_table = Table_Create();
    new_table->capacity = table->capacity == 0 ? TABLE_INITIAL_CAPACITY : table->capacity * TABLE_GROWTH_FACTOR;
    new_table->slots = malloc(new_table->capacity * sizeof(TableSlot));
    if (!new_table->slots) {
        Table_Destroy(table);
        Table_Destroy(new_table);
        logFatal("Failed to allocate memory for table slots.");
    }

    // init new table slots
    for (size_t i=0; i<new_table->capacity; i++) {
        TableSlot_Init(&new_table->slots[i]);
    }

    // copy old slots to new table
    shallow_copy_table(new_table, table);

    // free old table keys
    for (size_t i=0; i<table->capacity; i++) {
        TableSlot *slot = &table->slots[i];
        if (slot->state == TABLE_SLOT_USED) {
            table->key_free_func(slot->key);
        }
    }
    // free old table slots
    free(table->slots);
    
    // insert new table into original table
    table->slots = new_table->slots;
    table->capacity = new_table->capacity;
    table->used = new_table->used;

    // free new table shallow
    // Table_Destroy() is not used, cause the ownership of slots where transfered
    // and the keys where freed seperately 
    free(new_table);
}

void Table_Init(Table *table) {
    if (!table) {
        return;
    }
    table->slots = NULL;
    table->capacity = 0;
    table->used = 0;
}

void Table_Deinit(Table *table) {
    if (!table) {
        return;
    }
    if (table->slots) {
        for (size_t i=0; i<table->capacity; i++) {
            TableSlot *entry = &table->slots[i];
            TableSlot_Deinit(entry, table->key_free_func);
        }
        free(table->slots);
    }
    table->slots = NULL;
    table->capacity = 0;
    table->used = 0;
}

Table *Table_Create() {
    return Table_CreateCustom(
        (uint32_t(*)(const void*))hash_string,
        (int(*)(const void*, const void*))strcmp,
        (void*(*)(const void*))strdup, 
        (void(*)(void*))free);
}

Table *Table_CreatePtr() {
    return Table_CreateCustom(
        hash_ptr,
        cmp_ptr,
        cpy_ptr,
        NULL);
}

Table *Table_CreateCustom(
    uint32_t (*hash_func)(const void *p),
    int (*key_cmp_func)(const void*, const void *),
    void *(*key_copy_func)(const void *p),
    void (*key_free_func)(void *p)
) {
    Table *table = malloc(sizeof(Table));
    if (!table) {
        logFatal("Failed to allocate memory for table.");
    }
    Table_Init(table);
    table->hash_func = hash_func;
    table->key_cmp_func = key_cmp_func;
    table->key_copy_func = key_copy_func;
    table->key_free_func = key_free_func;
    return table;
}

void Table_Destroy(Table *table) {
    if (!table) {
        return;
    }
    Table_Deinit(table);
    free(table);
}


void Table_Set(Table *table, const void *key, void *value, void (*destructor)(void *value)) {
    if (!table || !key) {
        return;
    }
    if (table->used >= table->capacity * TABLE_MAX_LOAD_FACTOR) {
        increase_capacity(table);
    }
    TableSlot *slot = find_slot(table, key);
    if (!slot) {
        logFatal("No slot found in Table_Set().");
    }

    if (slot->state == TABLE_SLOT_EMPTY) {
        slot->state = TABLE_SLOT_USED;
        slot->key = table->key_copy_func(key);
        if (slot->key == NULL) {
            logFatal("No memory for string copy in Table_Set().");
        }
        table->used++;
    }
    // if table has the ownership of the value free it before overwriting it
    if (slot->destructor) {
        slot->destructor(slot->value);
    }
    slot->destructor = destructor;
    slot->value = value;  // potential pointers in value are copied and ownership is taken
}

void *Table_Get(const Table *table, const void *key) {
    if (!table) {
        logFatal("Invalid table in Table_Get().");
    }
    if (table->used == 0) {
        return NULL;
    }
    if (!key) {
        logWarn("Table_Get() was called with key == NULL.");
        return NULL;
    }

    TableSlot *slot = find_slot(table, key);
    if (slot->state == TABLE_SLOT_EMPTY) {
        return NULL;
    }
    return slot->value;
}

void Table_Delete(Table *table, const void *key) {
    if (!table) {
        logFatal("Invalid table in Table_Delete().");
    }
    if (table->used == 0) {
        return;
    }
    if (!key) {
        logWarn("Table_Delete() is called with key == NULL.");
        return;
    }
    TableSlot *slot = find_slot(table, key);
    if (slot->state != TABLE_SLOT_USED) {
        return;
    }
    TableSlot_Deinit(slot, table->key_free_func);
    slot->state = TABLE_SLOT_TOMBSTONE;

    // used is not decremented intentionally!
    // tombstones are counted to trigger rehash earlier and decrease probing
}

bool Table_Has(const Table *table, const void *key) {
    if (!table) {
        logFatal("Invalid table in Table_Has().");
    }
    if (table->used == 0) {
        return false;
    }
    if (!key) {
        logWarn("Table_Has() was called with key == NULL.");
        return false;
    }
    TableSlot *slot = find_slot(table, key);
    return (slot->state == TABLE_SLOT_USED);
}

bool Table_HasOwnership(const Table *table, const void *key) {
    if (!table) {
        logFatal("Invalid table in Table_HasOwnership().");
    }
    if (table->used == 0) {
        return false;
    }
    if (!key) {
        logWarn("Table_HasOwnership() was called with key == NULL.");
        return false;
    }
    TableSlot *slot = find_slot(table, key);
    return (slot->state == TABLE_SLOT_USED && slot->destructor != NULL);
}

size_t Table_GetUsage(const Table *table) {
    if (!table) {
        return 0;
    }
    return table->used;
}