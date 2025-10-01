#ifndef CONFIG_H
#define CONFIG_H

void Config_Init();
void Config_Deinit();

#define FILENAME_MAX_LENGTH 32

void Config_SetFilename(const char *filename);
const char* Config_GetFilename();


#endif