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