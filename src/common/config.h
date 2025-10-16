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
#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>
#include <stdbool.h>
#include "common/typedtable.h"


void Config_Init();
void Config_Deinit();

void Config_LoadIni(const char *content);

bool Config_IsDirty();
void Config_Loaded();

#define FILENAME_MAX_LENGTH 256

void Config_SetFilename(const char *filename);
const char* Config_GetFilename();

Table *Config_GetModuleConfig(const char *section);
int Config_GetNumber(Table *table, const char *key, int fallback);
const char *Config_GetStr(Table *table, const char *key, const char *fallback);
uint8_t Config_GetColor(Table *table, const char *key, uint8_t fallback);


#endif