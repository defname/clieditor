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