#include "table.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "logging.h"


void free_value(TableValue *value, TableValueType type) {
    if (!value) {
        return;
    }
    if (type == TABLE_VALUE_TYPE_STRING) {
        free(value->string_value);
    }
    if (type == TABLE_VALUE_TYPE_TABLE) {
        Table_Destroy(value->table_value);
    }
}

void TableEntry_Init(TableSlot *slot) {
    slot->key = NULL;
    slot->value.int_value = 0;
    slot->value.string_value = NULL;
    slot->value.table_value = NULL;
    slot->type = TABLE_VALUE_TYPE_NONE;
    slot->state = TABLE_SLOT_EMPTY;
}

void TableEntry_Deinit(TableSlot *slot) {
    if (!slot) {
        return;
    }
    if (slot->key) {
        free(slot->key);
    }
   free_value(&slot->value, slot->type);
}

// djb2
static uint32_t hash_string(const char *str) {
    uint32_t hash = 5381;
    char c;

    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash;
}

static TableSlot *find_slot(Table *table, const char *key) {
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

// copy the entries of src to the dst. do noch copy pointer targets only pointers itself
static void shallow_copy_table(Table *dst, const Table *src) {
    if (!dst || !src) {
        return;
    }
    if (dst->capacity < src->capacity) {
        logFatal("Wrong use of copy_table. (dst->capacity < src->capacity)");
    }
    size_t used = 0;
    for (int i=0; i<src->capacity; i++) {
        TableSlot *src_slot = &src->slots[i];
        if (src_slot->state != TABLE_SLOT_USED) {
            continue;
        }
        TableSlot *dst_slot = find_slot(dst, src_slot->key);
        // Table_Set() copies the key but not the char* string or a Table* in value, only the pointers)
        Table_Set(dst, src_slot->key, src_slot->type, src_slot->value);
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
    for (int i=0; i<new_table->capacity; i++) {
        TableEntry_Init(&new_table->slots[i]);
    }

    // copy old slots to new table
    shallow_copy_table(new_table, table);

    // free old table keys
    for (int i=0; i<table->capacity; i++) {
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
        for (int i=0; i<table->capacity; i++) {
            TableSlot *entry = &table->slots[i];
            TableEntry_Deinit(entry);
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


void Table_Set(Table *table, const char*key, TableValueType type, TableValue value) {
    if (!table || !key) {
        return;
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
    // free potential pointers before overwriting them
    free_value(&slot->value, slot->type);
    slot->type = type;
    slot->value = value;  // potential pointers in value are copied and ownership is taken
}

TableValueType Table_Get(Table *table, const char *key, TableValue *value, TableValue fallback) {
    if (!table) {
        logFatal("Invalid table in Table_Get().");
    }
    if (!key) {
        logWarn("Table_Get() was called with key == NULL.");
        *value = fallback;
        return TABLE_VALUE_TYPE_NONE;
    }

    TableSlot *slot = find_slot(table, key);
    if (slot->state == TABLE_SLOT_EMPTY) {
        *value = (TableValue){ 0 };
        return TABLE_VALUE_TYPE_NONE;
    }
    *value = slot->value;
    return slot->type;
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
    free(slot->key);
    slot->key = NULL;
    free_value(&slot->value, slot->type);
    slot->type = TABLE_VALUE_TYPE_NONE;;
    slot->value = (TableValue){ 0 };
    slot->state = TABLE_SLOT_TOMBSTONE;
}

void Table_SetInt(Table *table, const char *key, int value) {
    Table_Set(table, key, TABLE_VALUE_TYPE_INT, (TableValue){ .int_value = value });
}

void Table_SetStr(Table *table, const char *key, const char *value) {
    Table_Set(table, key, TABLE_VALUE_TYPE_STRING, (TableValue){ .string_value = value });
}

void Table_SetTable(Table *table, const char *key, Table *value) {
    Table_Set(table, key, TABLE_VALUE_TYPE_TABLE, (TableValue){ .table_value = value });
}

