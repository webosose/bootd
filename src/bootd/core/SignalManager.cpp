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

#include <wait.h>

#include "util/Logger.h"

#include "SignalManager.h"

const char *SignalManager::CORE_BOOT_DONE = "core-boot-done";
const char *SignalManager::INIT_BOOT_DONE = "init-boot-done";
const char *SignalManager::DATASTORE_INIT_START = "datastore-init-start";
const char *SignalManager::MINIMAL_BOOT_DONE = "minimal-boot-done";
const char *SignalManager::REST_BOOT_DONE = "rest-boot-done";
const char *SignalManager::BOOT_DONE = "boot-done";

const char *SignalManager::BOOTMODE_NORMAL_INIT_BOOT_DONE = "normal-init-boot-done";
const char *SignalManager::BOOTMODE_NORMAL_BOOT_DONE = "normal-boot-done";

SignalManager::SignalManager()
{
    m_oneshotSignals.clear();
    m_periodicSignals.clear();

    m_oneshotSignals[CORE_BOOT_DONE] = false;
    m_oneshotSignals[INIT_BOOT_DONE] = false;
    m_oneshotSignals[DATASTORE_INIT_START] = false;
    m_oneshotSignals[MINIMAL_BOOT_DONE] = false;
    m_oneshotSignals[REST_BOOT_DONE] = false;
    m_oneshotSignals[BOOT_DONE] = false;
}

SignalManager::~SignalManager()
{
    m_oneshotSignals.clear();
    m_periodicSignals.clear();
}

bool SignalManager::isGenerated(string name)
{
    for (size_t i = 0; i < m_periodicSignals.size(); i++) {
        if (m_periodicSignals.at(i) == name) {
            return false;
        }
    }
    if (m_oneshotSignals.find(name) == m_oneshotSignals.end()) {
        g_Logger.errorLog(Logger::MSGID_UPSTART, "The '%s' is not in map", name.c_str());
        return true;
    }
    return m_oneshotSignals.find(name)->second;
}

bool SignalManager::generate(string name, const bool isSync)
{
    if (isGenerated(name)) {
        g_Logger.warningLog(Logger::MSGID_UPSTART, "The '%s' is already generated", name.c_str());
        return true;
    }
    string command = "/sbin/initctl emit ";

    if (!isSync) {
        command += "--no-wait ";
    }
    command += name;
    g_Logger.performanceLog(Logger::MSGID_BOOT, "SIGNAL_%s", name.c_str());

    if (-1 == system(command.c_str())) {
        g_Logger.warningLog(Logger::MSGID_UPSTART, "Generating '%s' fails", name.c_str());
        return false;
    }

    auto signal = m_oneshotSignals.find(name);
    if (signal != m_oneshotSignals.end())
        m_oneshotSignals.find(name)->second = true;
    return true;
}

bool SignalManager::startBootMode(string mode, const bool isSync)
{
    char *command[] = {
        (char*)"/sbin/initctl",
        (char*)"start",
        (char*)"bootd-mode",
        NULL,
        NULL,
        NULL
    };
    string bootmode = "BOOTMODE=";

    bootmode += mode;
    command[3] = (char*)bootmode.c_str();
    command[4] = (isSync ? NULL : (char*)"--no-wait");

    pid_t pid = fork();
    if (pid == -1) {
        g_Logger.errorLog(Logger::MSGID_UPSTART, "Fork fails ==> startBootMode fails.");
        return false;
    }
    else if(pid == 0){
        execv(command[0], command);
        g_Logger.errorLog(Logger::MSGID_UPSTART, "Execv fails ==> startBootMode fails");
        exit(1);
    }
    int status;
    waitpid(pid, &status, 0);
    g_Logger.performanceLog(Logger::MSGID_BOOT, "SIGNAL_%s", bootmode.c_str());
    return true;
}

JValue SignalManager::getSignals()
{
    JValue signals = pbnjson::Object();
    for (auto it = m_oneshotSignals.begin(); it != m_oneshotSignals.end(); ++it) {
        signals.put(it->first, it->second);
    }
    return signals;
}
