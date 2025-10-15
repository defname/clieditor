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

void TableSlot_Deinit(TableSlot *slot) {
    if (!slot) {
        return;
    }
    if (slot->key) {
        free(slot->key);
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

static TableSlot *find_slot(const Table *table, const char *key) {
    if (!table || !key) {
        logFatal("Invalid table or key in find_slot()");
    }
    if (table->used >= table->capacity * TABLE_MAX_LOAD_FACTOR) {
        logFatal("Table too loaded.");  // this can only happen there is an error in the code
    }
    uint32_t hash = hash_string(key);
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
        else if (slot->state == TABLE_SLOT_USED && strcmp(slot->key, key) == 0) {
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
            free(slot->key);
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
            TableSlot_Deinit(entry);
        }
        free(table->slots);
    }
    table->slots = NULL;
    table->capacity = 0;
    table->used = 0;
}

Table *Table_Create() {
    Table *table = malloc(sizeof(Table));
    Table_Init(table);
    return table;
}

void Table_Destroy(Table *table) {
    if (!table) {
        return;
    }
    Table_Deinit(table);
    free(table);
}


void Table_Set(Table *table, const char*key, void *value, void (*destructor)(void *value)) {
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
        slot->key = strdup(key);
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

void *Table_Get(const Table *table, const char *key) {
    if (!table) {
        logFatal("Invalid table in Table_Get().");
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

void Table_Delete(Table *table, const char *key) {
    if (!table) {
        logFatal("Invalid table in Table_Delete().");
    }
    if (!key) {
        logWarn("Table_Delete() is called with key == NULL.");
        return;
    }
    TableSlot *slot = find_slot(table, key);
    if (slot->state != TABLE_SLOT_USED) {
        return;
    }
    TableSlot_Deinit(slot);
    slot->state = TABLE_SLOT_TOMBSTONE;

    // used is not decremented intentionally!
    // tombstones are counted to trigger rehash earlier and decrease probing
}

bool Table_Has(const Table *table, const char *key) {
    if (!table) {
        logFatal("Invalid table in Table_Has().");
    }
    if (!key) {
        logWarn("Table_Has() was called with key == NULL.");
        return false;
    }
    TableSlot *slot = find_slot(table, key);
    return (slot->state == TABLE_SLOT_USED);
}

bool Table_HasOwnership(const Table *table, const char *key) {
    if (!table) {
        logFatal("Invalid table in Table_HasOwnership().");
    }
    if (!key) {
        logWarn("Table_HasOwnership() was called with key == NULL.");
        return false;
    }
    TableSlot *slot = find_slot(table, key);
    return (slot->state == TABLE_SLOT_USED && slot->destructor != NULL);
}
