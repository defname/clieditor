#ifndef LOGGING_H
#define LOGGING_H

#define DEBUG

typedef enum {
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARN,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_FATAL
} LogLevel;


#define LOG_LEVEL LOG_LEVEL_DEBUG


void logger(LogLevel level, const char* format, ...);

void _logDebug(const char* format, ...);
void _logInfo(const char* format, ...);
void _logWarn(const char* format, ...);
void _logError(const char* format, ...);
void _logFatal(const char* format, ...);

#ifdef DEBUG
#define logDebug(fmt, ...) _logDebug("\n -- [%s:%d] %s\n -- " fmt, __FILE__, __LINE__, __func__, ##__VA_ARGS__)
#define logInfo(fmt, ...) _logInfo("\n -- [%s:%d] %s\n -- " fmt, __FILE__, __LINE__, __func__, ##__VA_ARGS__)
#define logWarn(fmt, ...) _logWarn("\n -- [%s:%d] %s\n -- " fmt, __FILE__, __LINE__, __func__, ##__VA_ARGS__)
#define logError(fmt, ...) _logError("\n -- [%s:%d] %s\n -- " fmt, __FILE__, __LINE__, __func__, ##__VA_ARGS__)
#define logFatal(fmt, ...) _logFatal("\n -- [%s:%d] %s\n -- " fmt, __FILE__, __LINE__, __func__, ##__VA_ARGS__)
#else
#define logDebug(fmt, ...) _logDebug(fmt, ##__VA_ARGS__)
#define logInfo(fmt, ...) _logInfo(fmt, ##__VA_ARGS__)
#define logWarn(fmt, ...) _logWarn(fmt, ##__VA_ARGS__)
#define logError(fmt, ...) _logError(fmt, ##__VA_ARGS__)
#define logFatal(fmt, ...) _logFatal(fmt, ##__VA_ARGS__)
#endif

#endif