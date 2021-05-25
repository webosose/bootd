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

#ifndef LOGGER_H
#define LOGGER_H

#include <stdlib.h>
#include <time.h>

#include <iostream>

#include <PmLogLib.h>

using namespace std;

enum LogType {
    logToNull,
    logToBootdLog,
    logToPmLog,
    logToConsoleLog
};

enum LogLevel {
    unknown,
    debug,
    performance,
    info,
    warning,
    error
};

class Logger {
public:
    Logger();
    ~Logger();
    void initLogFile();

    void debugLog(const char *msgid, const char *format, ...);
    void performanceLog(const char *msgid, const char *format, ...);
    void infoLog(const char *msgid, const char *format, ...);
    void warningLog(const char *msgid, const char *format, ...);
    void errorLog(const char *msgid, const char *format, ...);

    bool getCurrentTime(struct timespec &curTime);
    void getBaseTime(struct timespec &time);

    bool enableLogType(enum LogType newType);
    bool changeLogLevel(enum LogLevel newLevel);
    enum LogType getLogType(string type);
    enum LogLevel getLogLevel(string level);

    static const char *PERFORMANCE_LOGFILE_NAME;

    static const char *MSGID_ACTION;
    static const char *MSGID_BOOT;
    static const char *MSGID_BOOTSEQUENCER;
    static const char *MSGID_CLIENT;
    static const char *MSGID_CONDITION;
    static const char *MSGID_CONTROLLER;
    static const char *MSGID_EVENT;
    static const char *MSGID_GENERAL;
    static const char *MSGID_PRIORITY;
    static const char *MSGID_SERVICE;
    static const char *MSGID_SETTINGS;
    static const char *MSGID_UPSTART;
    static const char *MSGID_UTIL;

private:
    static bool getSystemTime(struct timespec *ts);
    static bool getHWTime(struct timespec *ts);
    static void subtractTimespec(struct timespec *result,
                                 const struct timespec *time1,
                                 const struct timespec *time2);

    PmLogContext getLogContext();
    void writeBootdLog(const char *type, const char *msgid, const char *msg);

    static const int MAX_LOG_BUFFER = 512;
    static const int DEFAULT_ROTATION_COUNT = 10;

    bool m_enableBootdLog;
    bool m_enablePmLog;
    bool m_enableConsoleLog;

    enum LogLevel m_logLevel;

    PmLogContext m_context;
    char m_msgBuffer[MAX_LOG_BUFFER];
    int m_fdBootdLog;

    struct timespec m_baseTime;

};

extern Logger g_Logger;

#endif // LOGGER_H
