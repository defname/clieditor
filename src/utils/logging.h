#ifndef LOGGING_H
#define LOGGING_H

typedef enum {
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARN,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_FATAL
} LogLevel;


#define LOG_LEVEL LOG_LEVEL_DEBUG


void logger(LogLevel level, const char* format, ...);

void logDebug(const char* format, ...);
void logInfo(const char* format, ...);
void logWarn(const char* format, ...);
void logError(const char* format, ...);
void logFatal(const char* format, ...);

#endif