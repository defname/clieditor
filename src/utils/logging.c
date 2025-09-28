#include "logging.h"

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

void logger_v(LogLevel level, const char* format, va_list args) {
#ifdef LOG_LEVEL
    if (level < LOG_LEVEL) {
        return;
    }
#endif
    const char* levelStr;
    switch (level) {
        case LOG_LEVEL_DEBUG: levelStr = "DEBUG"; break;
        case LOG_LEVEL_INFO:  levelStr = "INFO";  break;
        case LOG_LEVEL_WARN:  levelStr = "WARN";  break;
        case LOG_LEVEL_ERROR: levelStr = "ERROR"; break;
        case LOG_LEVEL_FATAL: levelStr = "FATAL"; break;
        default:              levelStr = "UNKNOWN"; break;
    }

    time_t now = time(NULL);
    struct tm* t = localtime(&now);
    char timeStr[20];
    strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", t);

    fprintf(stderr, "[%s] [%s] ", timeStr, levelStr);
    vfprintf(stderr, format, args);
    fprintf(stderr, "\n");

    if (level == LOG_LEVEL_FATAL) {
        exit(1);
    }
}

void logger(LogLevel level, const char* format, ...) {
    va_list args;
    va_start(args, format);
    logger_v(level, format, args);
    va_end(args);
}

#define DEFINE_LOGGER(levelFunc, level) \
void levelFunc(const char* format, ...) { \
    va_list args; \
    va_start(args, format); \
    logger_v((level), format, args); \
    va_end(args); \
}

DEFINE_LOGGER(_logDebug, LOG_LEVEL_DEBUG)
DEFINE_LOGGER(_logInfo, LOG_LEVEL_INFO)
DEFINE_LOGGER(_logWarn, LOG_LEVEL_WARN)
DEFINE_LOGGER(_logError, LOG_LEVEL_ERROR)
DEFINE_LOGGER(_logFatal, LOG_LEVEL_FATAL)
