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

void logDebugImpl(const char* format, ...);
void logInfoImpl(const char* format, ...);
void logWarnImpl(const char* format, ...);
void logErrorImpl(const char* format, ...);
void logFatalImpl(const char* format, ...);

#ifdef DEBUG
#define logDebug(fmt, ...) logDebugImpl("\n -- [%s:%d] %s\n -- " fmt, __FILE__, __LINE__, __func__, ##__VA_ARGS__)
#define logInfo(fmt, ...) logInfoImpl("\n -- [%s:%d] %s\n -- " fmt, __FILE__, __LINE__, __func__, ##__VA_ARGS__)
#define logWarn(fmt, ...) logWarnImpl("\n -- [%s:%d] %s\n -- " fmt, __FILE__, __LINE__, __func__, ##__VA_ARGS__)
#define logError(fmt, ...) logErrorImpl("\n -- [%s:%d] %s\n -- " fmt, __FILE__, __LINE__, __func__, ##__VA_ARGS__)
#define logFatal(fmt, ...) logFatalImpl("\n -- [%s:%d] %s\n -- " fmt, __FILE__, __LINE__, __func__, ##__VA_ARGS__)
#else
#define logDebug(fmt, ...) logDebugImpl(fmt, ##__VA_ARGS__)
#define logInfo(fmt, ...) logInfoImpl(fmt, ##__VA_ARGS__)
#define logWarn(fmt, ...) logWarnImpl(fmt, ##__VA_ARGS__)
#define logError(fmt, ...) logErrorImpl(fmt, ##__VA_ARGS__)
#define logFatal(fmt, ...) logFatalImpl(fmt, ##__VA_ARGS__)
#endif

#endif