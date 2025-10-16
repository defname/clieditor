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
#include "common/typedtable.h"
#include "common/iniparser.h"

typedef struct _Config {
    Table *table;
    const Table *editor;
    const Table *colors;

    int indent_size;
    bool use_spaces_for_indent;
    char filename[FILENAME_MAX_LENGTH];
} Config;

static Config config;

void Config_Init() {
    config.indent_size = 4;
    config.use_spaces_for_indent = true;
    config.filename[0] = '\0';
    config.table = NULL;
    config.editor = NULL;
    config.colors = NULL;
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
    config.table = c;
    config.editor = TypedTable_GetTable(config.table, "editor");
    config.colors = TypedTable_GetTable(config.table, "colors");
}

void Config_SetFilename(const char *filename) {
    if (filename == NULL) {
        config.filename[0] = '\0';
        return;
    }
    strncpy(config.filename, filename, FILENAME_MAX_LENGTH);
    config.filename[FILENAME_MAX_LENGTH - 1] = '\0';
}

const char *Config_GetFilename() {
    return config.filename;
}


int Config_GetNumber(const char *key, int fallback) {
    if (TypedTable_GetType(config.editor, key) != VALUE_TYPE_NUMBER) {
        return fallback;
    }
    return TypedTable_GetNumber(config.editor, key);
}

const char *Config_GeStr(const char *key, const char *fallback) {
    if (TypedTable_GetType(config.editor, key) != VALUE_TYPE_STRING) {
        return fallback;
    }
    return TypedTable_GetString(config.editor, key);
}

uint8_t Config_GetColor(const char *key, uint8_t fallback) {
    if (TypedTable_GetType(config.colors, key) != VALUE_TYPE_NUMBER) {
        return fallback;
    }
    return (uint8_t)TypedTable_GetNumber(config.colors, key);
}