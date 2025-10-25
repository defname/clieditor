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
#include "config.h"

#include <stdbool.h>
#include <string.h>
#include <limits.h>     // PATH_MAX
#include "common/typedtable.h"
#include "common/iniparser.h"

typedef struct _Config {
    Table *table;
    const Table *editor;
    const Table *colors;

    bool dirty;

    const char *exe_path;

    int indent_size;
    bool use_spaces_for_indent;
    char filename[PATH_MAX];
} Config;

static Config config;

void Config_Init(const char *argv0) {
    config.exe_path = argv0;
    config.indent_size = 4;
    config.use_spaces_for_indent = true;
    config.filename[0] = '\0';
    config.table = NULL;
    config.editor = NULL;
    config.colors = NULL;
    config.dirty = false;
}

void Config_Deinit() {
    if (config.table) {
        Table_Destroy(config.table);
    }
    config.table = NULL;
    config.editor = NULL;
    config.colors = NULL;
}


void Config_LoadIni(const char *content) {
    if (!content) {
        return;
    }
    IniParser ini;
    IniParser_Init(&ini);
    IniParser_SetText(&ini, content);
    Table *c = IniParser_Parse(&ini);
    if (!c) {
        return;
    }
    if (config.table) {
        Table_Destroy(config.table);
    }
    config.table = c;
    config.editor = TypedTable_GetTable(config.table, "editor");
    config.colors = TypedTable_GetTable(config.table, "colors");
    config.dirty = true;
}

bool Config_IsDirty() {
    return config.dirty;
}

void Config_Loaded() {
    config.dirty = false;
}

const char *Config_GetExePath() {
    return config.exe_path;
}

void Config_SetFilename(const char *filename) {
    if (filename == NULL) {
        config.filename[0] = '\0';
        return;
    }
    strncpy(config.filename, filename, PATH_MAX - 1);
    config.filename[PATH_MAX - 1] = '\0';
}

const char *Config_GetFilename() {
    return config.filename;
}

Table *Config_GetModuleConfig(const char *section) {
    if (!config.table || TypedTable_GetType(config.table, section) != VALUE_TYPE_TABLE) {
        return NULL;
    }
    return TypedTable_GetTable(config.table, section);
}

int Config_GetNumber(Table *table, const char *key, int fallback) {
    if (!table || TypedTable_GetType(table, key) != VALUE_TYPE_NUMBER) {
        return fallback;
    }
    return TypedTable_GetNumber(table, key);
}

const char *Config_GetStr(Table *table, const char *key, const char *fallback) {
    if (!table || TypedTable_GetType(table, key) != VALUE_TYPE_STRING) {
        return fallback;
    }
    return TypedTable_GetString(table, key);
}

uint8_t Config_GetColor(Table *table, const char *key, uint8_t fallback) {
    if (!table || TypedTable_GetType(table, key) != VALUE_TYPE_NUMBER) {
        return fallback;
    }
    return (uint8_t)TypedTable_GetNumber(table, key);
}