#include "typedtable.h"
#include <string.h>
#include "common/logging.h"


TypedValue *TypedValue_Create() {
    TypedValue *value = malloc(sizeof(TypedValue));
    if (!value) {
        logFatal("Cannot allocate memory for TypedValue.");
    }
    value->type = VALUE_TYPE_NONE;
    value->data.number_value = 0;
    return value;
}

void TypedValue_Destroy(TypedValue *value) {
    if (!value) {
        return;
    }
    switch (value->type) {
        case VALUE_TYPE_NONE:
        case VALUE_TYPE_NUMBER:
        case VALUE_TYPE_BOOLEAN:
            break;
        case VALUE_TYPE_STRING:
            if (value->data.string_value) {
                free(value->data.string_value);
            }
            break;
        case VALUE_TYPE_TABLE:
            if (value->data.table_value) {
                Table_Destroy(value->data.table_value);
            }
            break;
    }
    free(value);
}

void TypedTable_SetNumber(Table *table, const char *key, int value) {
    TypedValue *typed_value = TypedValue_Create();
    typed_value->type = VALUE_TYPE_NUMBER;
    typed_value->data.number_value = value;
    Table_Set(table, key, typed_value, (void(*)(void*))TypedValue_Destroy);
}


void TypedTable_SetString(Table *table, const char *key, char *value) {
    TypedValue *typed_value = TypedValue_Create();
    typed_value->type = VALUE_TYPE_STRING;
    typed_value->data.string_value = value;
    Table_Set(table, key, typed_value, (void(*)(void*))TypedValue_Destroy);
}

void TypedTable_SetStringCopy(Table *table, const char *key, const char *value) {
    char *copy = strdup(value);
    if (!copy) {
        logFatal("Cannot allocate memory for string in TypedTable_SetStringCopy().");
    }
    TypedTable_SetString(table, key, copy);
}

void TypedTable_SetBoolean(Table *table, const char *key, bool value) {
    TypedValue *typed_value = TypedValue_Create();
    typed_value->type = VALUE_TYPE_BOOLEAN;
    typed_value->data.boolean_value = value;
    Table_Set(table, key, typed_value, (void(*)(void*))TypedValue_Destroy);
}
void TypedTable_SetTable(Table *table, const char *key, Table *value) {
    if (value == table) {
        logFatal("Table cannot contain itself as value.");
    }
    if (!value) {
        logFatal("Table cannot be NULL in TypedTable_SetTable().");
    }
    TypedValue *typed_value = TypedValue_Create();
    typed_value->type = VALUE_TYPE_TABLE;
    typed_value->data.table_value = value;
    Table_Set(table, key, typed_value, (void(*)(void*))TypedValue_Destroy);
}

ValueType TypedTable_GetType(const Table *table, const char *key) {
    TypedValue *typed_value = Table_Get(table, key);
    if (!typed_value) {
        return VALUE_TYPE_NONE;
    }
    return typed_value->type;
}

int TypedTable_GetNumber(const Table *table, const char *key) {
    TypedValue *typed_value = Table_Get(table, key);
    if (!typed_value || typed_value->type != VALUE_TYPE_NUMBER) {
        if (typed_value && typed_value->type != VALUE_TYPE_NUMBER) {
            logWarn("%s: key %s has wrong type.", __func__, key);
        }
        return 0;
    }
    return typed_value->data.number_value;
}

const char* TypedTable_GetString(const Table *table, const char *key) {
    TypedValue *typed_value = Table_Get(table, key);
    if (!typed_value || typed_value->type != VALUE_TYPE_STRING) {
        if (typed_value && typed_value->type != VALUE_TYPE_STRING) {
            logWarn("%s: key %s has wrong type.", __func__, key);
        }
        return NULL;
    }
    return typed_value->data.string_value;
}

bool TypedTable_GetBoolean(const Table *table, const char *key) {
    TypedValue *typed_value = Table_Get(table, key);
    if (!typed_value || typed_value->type != VALUE_TYPE_BOOLEAN) {
        if (typed_value && typed_value->type != VALUE_TYPE_BOOLEAN) {
            logWarn("%s: key %s has wrong type.", __func__, key);
        }
        return false;
    }
    return typed_value->data.boolean_value;
}

Table* TypedTable_GetTable(const Table *table, const char *key) {
    TypedValue *typed_value = Table_Get(table, key);
    if (!typed_value || typed_value->type != VALUE_TYPE_TABLE) {
        if (typed_value && typed_value->type != VALUE_TYPE_BOOLEAN) {
            logWarn("%s: key %s has wrong type.", __func__, key);
        }
        return NULL;
    }
    return typed_value->data.table_value;
}
