// Copyright (c) 2013-2021 LG Electronics, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// SPDX-License-Identifier: Apache-2.0

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>

#include "event/StaticEventDB.h"
#include "Main.h"

#include "Logger.h"

Logger g_Logger;

const char *Logger::PERFORMANCE_LOGFILE_NAME = "/var/log/bootd.log";

const char *Logger::MSGID_ACTION = "Action";
const char *Logger::MSGID_BOOT = "Boot";
const char *Logger::MSGID_BOOTSEQUENCER = "BootSequencer";
const char *Logger::MSGID_CLIENT = "Client";
const char *Logger::MSGID_CONDITION = "Condition";
const char *Logger::MSGID_CONTROLLER = "Controller";
const char *Logger::MSGID_EVENT = "Event";
const char *Logger::MSGID_GENERAL = "General";
const char *Logger::MSGID_PRIORITY = "Priority";
const char *Logger::MSGID_SERVICE = "Service";
const char *Logger::MSGID_SETTINGS = "Settings";
const char *Logger::MSGID_UPSTART = "Upstart";
const char *Logger::MSGID_UTIL = "Utils";

bool Logger::getSystemTime(struct timespec *ts)
{
    if (clock_gettime(CLOCK_MONOTONIC, ts) == -1)
        return false;
    return true;
}

void Logger::subtractTimespec(struct timespec *result,
                              const struct timespec *time1,
                              const struct timespec *time2)
{
    result->tv_sec = time1->tv_sec - time2->tv_sec;
    result->tv_nsec = time1->tv_nsec - time2->tv_nsec;
    if (result->tv_nsec < 0) {
        result->tv_sec -= 1;
        result->tv_nsec += 1000000000;
    }
}

Logger::Logger()
    : m_enableBootdLog(true),
      m_enablePmLog(false),
      m_enableConsoleLog(false),
      m_logLevel(LogLevel::debug),
      m_fdBootdLog(-1)
{
    m_context = PmLogGetContextInline("bootManager");

    memset(m_msgBuffer, 0, MAX_LOG_BUFFER);
    initLogFile();
    struct timespec systemTime;

    // setting default time
    m_baseTime.tv_sec = 0;
    m_baseTime.tv_nsec = 0;

    getSystemTime(&systemTime);
}

Logger::~Logger()
{
    if (m_fdBootdLog >= 0) {
        close(m_fdBootdLog);
    }
}

void Logger::initLogFile()
{
    static int rotationCount = 0;

    if (rotationCount == 0) {
        if (m_fdBootdLog >= 0) {
            close(m_fdBootdLog);
        }
        m_fdBootdLog = open(PERFORMANCE_LOGFILE_NAME, O_RDWR | O_CREAT | O_TRUNC, 0600);
        if (m_fdBootdLog <= 0) {
            cerr << "File Open Error : " << PERFORMANCE_LOGFILE_NAME << endl;
        }
    }
    rotationCount++;
    if (rotationCount == DEFAULT_ROTATION_COUNT)
        rotationCount = 0;
}

void Logger::writeBootdLog(const char *type, const char *msgid, const char *msg)
{
    struct timespec curTime;

    if (m_fdBootdLog < 0)
        return;

    if (!getCurrentTime(curTime))
        return;

    dprintf(m_fdBootdLog,
            "%s(%3ld.%09ld) : %s) %s\n",
            type,
            curTime.tv_sec,
            curTime.tv_nsec,
            msgid,
            msg);
}

void Logger::performanceLog(const char *msgid, const char *format, ...)
{
    if (m_logLevel > LogLevel::performance)
        return;
    va_list argp;

    va_start(argp, format);
    vsnprintf(m_msgBuffer, MAX_LOG_BUFFER, format, argp);
    va_end(argp);

    if (m_enableBootdLog) {
        writeBootdLog("PERFORMANCE", msgid, m_msgBuffer);
    }
    if (m_enablePmLog) {
        PmLogInfo(m_context, msgid, 0, m_msgBuffer);
    }
    if (m_enableConsoleLog) {
        cout << "INFO :" << m_msgBuffer << std::endl;
    }
}

void Logger::debugLog(const char *msgid, const char *format, ...)
{
    if (m_logLevel > LogLevel::debug)
        return;
    va_list argp;

    va_start(argp, format);
    vsnprintf(m_msgBuffer, MAX_LOG_BUFFER, format, argp);
    va_end(argp);

    if (m_enableBootdLog) {
        writeBootdLog("DEBUG", msgid, m_msgBuffer);
    }
    if (m_enablePmLog) {
        PmLogDebug(m_context, m_msgBuffer);
    }
    if (m_enableConsoleLog) {
        cout << "DEBUG : " << m_msgBuffer << endl;
    }
}

void Logger::infoLog(const char *msgid, const char *format, ...)
{
    if (m_logLevel > LogLevel::info)
        return;
    va_list argp;

    va_start(argp, format);
    vsnprintf(m_msgBuffer, MAX_LOG_BUFFER, format, argp);
    va_end(argp);

    if (m_enableBootdLog) {
        writeBootdLog("INFO", msgid, m_msgBuffer);
    }
    if (m_enablePmLog) {
        PmLogInfo(m_context, msgid, 0, m_msgBuffer);
    }
    if (m_enableConsoleLog) {
        cout << "INFO :" << m_msgBuffer << std::endl;
    }
}

void Logger::warningLog(const char *msgid, const char *format, ...)
{
    if (m_logLevel > LogLevel::warning)
        return;
    va_list argp;

    va_start(argp, format);
    vsnprintf(m_msgBuffer, MAX_LOG_BUFFER, format, argp);
    va_end(argp);

    if (m_enableBootdLog) {
        writeBootdLog("WARNING", msgid, m_msgBuffer);
    }
    if (m_enablePmLog) {
        PmLogWarning(m_context, msgid, 0, m_msgBuffer);
    }
    if (m_enableConsoleLog) {
        cerr << "WARNING :" << m_msgBuffer << std::endl;
    }
}

void Logger::errorLog(const char *msgid, const char *format, ...)
{
    if (m_logLevel > LogLevel::error)
        return;
    va_list argp;

    va_start(argp, format);
    vsnprintf(m_msgBuffer, MAX_LOG_BUFFER, format, argp);
    va_end(argp);

    if (m_enableBootdLog) {
        writeBootdLog("ERROR", msgid, m_msgBuffer);
    }
    if (m_enablePmLog) {
        PmLogError(m_context, msgid, 0, m_msgBuffer);
    }
    if (m_enableConsoleLog) {
        cerr << "ERROR :" << m_msgBuffer << std::endl;
    }
}

bool Logger::getCurrentTime(struct timespec &curTime)
{
    struct timespec timeDiff;
    if (getSystemTime(&timeDiff) == false) {
        return false;
    }
    Logger::subtractTimespec(&curTime, &timeDiff, &m_baseTime);
    return true;
}

void Logger::getBaseTime(struct timespec &time)
{
    time = m_baseTime;
}

PmLogContext Logger::getLogContext()
{
    return m_context;
}

bool Logger::enableLogType(enum LogType type)
{
    if (LogType::logToBootdLog == type) {
        m_enableBootdLog = true;
    } else if (LogType::logToPmLog == type) {
        m_enablePmLog = true;
    } else if (LogType::logToConsoleLog == type) {
        m_enableConsoleLog = true;
    }
    return true;
}

enum LogType Logger::getLogType(string type)
{
    if ("file" == type) {
        return LogType::logToBootdLog;
    } else if ("pmlog" == type) {
        return LogType::logToPmLog;
    } else if ("console" == type) {
        return LogType::logToConsoleLog;
    } else {
        return LogType::logToNull;
    }
}

bool Logger::changeLogLevel(enum LogLevel newLevel)
{
    m_logLevel = newLevel;
    return true;
}

enum LogLevel Logger::getLogLevel(string level)
{
    if ("debug" == level) {
        return LogLevel::debug;
    } else if ("performance" == level) {
        return LogLevel::performance;
    } else if ("warning" == level) {
        return LogLevel::warning;
    } else if ("error" == level) {
        return LogLevel::error;
    } else {
        return LogLevel::unknown;
    }
}
