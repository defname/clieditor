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

typedef struct _Config {
    int indent_size;
    bool use_spaces_for_indent;
    char filename[FILENAME_MAX_LENGTH];
} Config;

static Config config;

void Config_Init() {
    config.indent_size = 4;
    config.use_spaces_for_indent = true;
    config.filename[0] = '\0';
}

void Config_Deinit() {
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