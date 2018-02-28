// Copyright (c) 2014-2018 LG Electronics, Inc.
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

#include <dirent.h>
#include <errno.h>
#include <fstream>
#include <stdio.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <unistd.h>

#include "SignalManager.h"
#include "util/Logger.h"

#include "PriorityManager.h"

PriorityManager::PriorityManager()
{

}

PriorityManager::~PriorityManager()
{

}

bool PriorityManager::getProcessInfo(string procStatPath, string &comm, struct ProcessInfo &processInfo)
{
    string pid, state, ppid, pgrp;

    try {
        ifstream ifile(procStatPath, ios::in);

        if (ifile.fail())
            return false;

        // /proc/<pid>/stat : "pid (comm) state ppid pgrp ..."
        ifile >> pid >> comm >> state >> ppid >> pgrp;

        processInfo.pid = atoi(pid.c_str());
        processInfo.ppid = atoi(ppid.c_str());
        processInfo.pgrp = atoi(pgrp.c_str());

        if (processInfo.pid == 0 || processInfo.pgrp == 0 || processInfo.ppid == 0)
            return false;

        // Should remove brackets
        comm = comm.substr(1, comm.length()-2);
    } catch (const std::exception& e) {
        g_Logger.errorLog(Logger::MSGID_PRIORITY, "Read process info failed=Message(%s)", e.what());
        return false;
    }

    return true;
}

bool PriorityManager::refreshProcessInfoMap()
{
    DIR *procDir;
    struct dirent *entry;
    string path;
    string procName;
    struct ProcessInfo processInfo;

    m_processInfoMap.clear();
    procDir = opendir("/proc/");
    if (procDir == NULL)
        return false;

    while (NULL != (entry = readdir(procDir))) {
        if (strspn(entry->d_name, "0123456789") == strlen(entry->d_name)) {
            path = "/proc/";
            path += entry->d_name;
            path += "/stat";

            if (!getProcessInfo(path, procName, processInfo)) {
                continue;
            }

            m_processInfoMap[procName] = processInfo;
        }
    }
    closedir(procDir);

    g_Logger.debugLog(Logger::MSGID_PRIORITY, "RefreshProcessInfoMap : Total(%d)", m_processInfoMap.size());

    return true;
}

bool PriorityManager::adjustProcessPriority(string processName, int nice)
{
    struct ProcessInfo processInfo;
    int returnValue;
    int i;

    // find process info
    processInfo.pid = 0;
    for (i = 0; i < RETRY_COUNT; i++) {
        auto it = m_processInfoMap.find(processName);
        if (it == m_processInfoMap.end()) {
            refreshProcessInfoMap();
            continue;
        }
        processInfo = it->second;
        break;
    }

    if (processInfo.pid == 0) {
        g_Logger.errorLog(Logger::MSGID_PRIORITY, "Cannot find process(%s) info", processName.c_str());
        return false;
    }

    // adjust nice value
    if (processInfo.pid == processInfo.pgrp)
        returnValue = setpriority(PRIO_PGRP, processInfo.pgrp, nice);
    else
        returnValue = setpriority(PRIO_PROCESS, processInfo.pid, nice);

    if (returnValue < 0) {
        g_Logger.errorLog(Logger::MSGID_PRIORITY,
                          "Cannot adjust process(%s) priority: %s",
                          processName.c_str(),
                          strerror(errno));
        return false;
    }

    g_Logger.debugLog(Logger::MSGID_PRIORITY, "Adjust process(%s) to priority(%d)", processName.c_str(), nice);
    return true;
}

void PriorityManager::printProcessInfoMap()
{
    g_Logger.debugLog(Logger::MSGID_PRIORITY, "printProcessInfoMap");
    for (auto i = m_processInfoMap.begin(); i != m_processInfoMap.end(); i++) {
        g_Logger.debugLog(Logger::MSGID_PRIORITY,
                          "%s - pid=%d ppid=%d pgrp=%d",
                          i->first.c_str(),
                          i->second.pid,
                          i->second.ppid,
                          i->second.pgrp);
    }
}

bool PriorityManager::adjustNiceValue(JValue priorities)
{
    string process;
    int value;
    for (ssize_t i = 0; i < priorities.arraySize(); i++) {
        process = priorities[i]["process"].asString();
        value = priorities[i]["value"].asNumber<int>();
        adjustProcessPriority(process, value);
    }
    return true;
}
