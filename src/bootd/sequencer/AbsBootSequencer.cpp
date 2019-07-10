// Copyright (c) 2016-2019 LG Electronics, Inc.
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

#include "core/SignalManager.h"
#include "util/Logger.h"

#include "AbsBootSequencer.h"

int AbsBootSequencer::_start(void* ctx)
{
    AbsBootSequencer *sequencer = (AbsBootSequencer *)ctx;
    sequencer->doBoot();
    return G_SOURCE_REMOVE;
}

void AbsBootSequencer::start()
{
    g_timeout_add_seconds(0, &AbsBootSequencer::_start, this);
    g_main_loop_run(m_mainLoop);
}

void AbsBootSequencer::stop()
{
    g_main_loop_quit(m_mainLoop);
}

string AbsBootSequencer::getBootStatusStr(BootStatus mode)
{
    switch (mode) {
    case BootStatus_normal:
        return "normal";
    default:
        return "unknown";
    }
}

string AbsBootSequencer::getBootTargetStr(BootTarget mode)
{
    switch(mode) {
    case BootTarget_emulator:
        return "emulator";

    case BootTarget_hardware:
        return "hardware";

    default:
        return "unknown";
    }
}

string AbsBootSequencer::getPowerStatusStr(PowerStatus mode)
{
    // There should not be whitespace
    // because LTTng doesn't support 'spaces'.
    switch(mode) {
    case PowerStatus_active:
        return "active";

    case PowerStatus_activeStandby:
        return "activeStandby";

    case PowerStatus_suspend:
        return "suspend";

    default:
        return "unknown";
    }
}

PowerStatus AbsBootSequencer::getPowerStatusType(string type)
{
    if (type == "Active") {
        return PowerStatus_active;
    } else if (type == "Active Standby") {
        return PowerStatus_activeStandby;
    } else if (type == "Suspend") {
        return PowerStatus_suspend;
    } else {
        return PowerStatus_unknown;
    }
}

AbsBootSequencer::AbsBootSequencer()
    : m_curBootStatus(BootStatus::BootStatus_unknown),
      m_curBootTarget(BootTarget::BootTarget_unknown),
      m_curPowerStatus(PowerStatus::PowerStatus_unknown)
{
    m_configuration = StaticEventDB::instance();
    m_configuration->printInformation();

    m_mainLoop = g_main_loop_new(NULL, FALSE);
}

AbsBootSequencer::~AbsBootSequencer()
{
    g_main_loop_unref(m_mainLoop);
}

string AbsBootSequencer::getBootStatus()
{
    return AbsBootSequencer::getBootStatusStr(m_curBootStatus);
}

string AbsBootSequencer::getBootTarget()
{
    return AbsBootSequencer::getBootTargetStr(m_curBootTarget);
}

string AbsBootSequencer::getPowerStatus()
{
    return AbsBootSequencer::getPowerStatusStr(m_curPowerStatus);
}

void AbsBootSequencer::proceedCoreBootDone()
{
    generateUpstartSignal(SignalManager::CORE_BOOT_DONE);
}

void AbsBootSequencer::proceedInitBootDone()
{
    generateUpstartSignal(SignalManager::INIT_BOOT_DONE);
}

void AbsBootSequencer::proceedDataStoreInitStart()
{
    generateUpstartSignal(SignalManager::DATASTORE_INIT_START);
}

void AbsBootSequencer::proceedMinimalBootDone()
{
    generateUpstartSignal(SignalManager::MINIMAL_BOOT_DONE);
}

void AbsBootSequencer::proceedRestBootDone()
{
    generateUpstartSignal(SignalManager::REST_BOOT_DONE);
}

void AbsBootSequencer::proceedBootDone()
{
    generateUpstartSignal(SignalManager::BOOT_DONE);
}

bool AbsBootSequencer::generateUpstartSignal(const char* signalName)
{
    if (SignalManager::instance()->isGenerated(signalName)) {
        g_Logger.warningLog(Logger::MSGID_BOOTSEQUENCER, "%s signal is already generated", signalName);
        return true;
    }
    SignalManager::instance()->generate(signalName);
    m_bootManager.postGetBootStatusSubscription(onGetBootStatus());
    return true;
}

JValue AbsBootSequencer::onGetBootStatus()
{
    pbnjson::JValue bootStatus = pbnjson::Object();
    bootStatus.put("bootStatus", getBootStatus());
    bootStatus.put("powerStatus", getPowerStatus());
    bootStatus.put("bootTarget", getBootTarget());
    bootStatus.put("signals", SignalManager::instance()->getSignals());
    return bootStatus;
}

bool AbsBootSequencer::onSAMStatusChange(bool isConnected)
{
    static bool serviceStatus = false;

    if (serviceStatus == isConnected) {
        g_Logger.debugLog(Logger::MSGID_BOOTSEQUENCER, "Same server status: '%s' (%s)",
                          ApplicationManager::instance()->getName().c_str(),
                          isConnected ? "up" : "down");
        return true;
    }
    serviceStatus = isConnected;

    if (!serviceStatus) {
        g_Logger.debugLog(Logger::MSGID_BOOTSEQUENCER, "'%s' is down", ApplicationManager::instance()->getName().c_str());
    } else {
        g_Logger.debugLog(Logger::MSGID_BOOTSEQUENCER, "'%s' is up", ApplicationManager::instance()->getName().c_str());
        ApplicationManager::instance()->getForegroundAppInfo(&m_bootManager, this);
    }
    return true;
}

void AbsBootSequencer::onForegroundAppChange(string &appId)
{
    if (appId.empty()) {
        g_Logger.debugLog(Logger::MSGID_BOOTSEQUENCER, "foreground is empty");
        return;
    }

    g_Logger.debugLog(Logger::MSGID_BOOTSEQUENCER, "'%s' is foreground", appId.c_str());

    if (DynamicEventDB::instance()->isWaitSomething()) {
        // We don't need to notify 'foreground' event if EventCore doesn't wait it.
        DynamicEventDB::instance()->triggerEvent(appId);
    }

    if (!DynamicEventDB::instance()->getEventStatus(DynamicEventDB::EVENT_FIRSTAPP_LAUNCHED)) {
        DynamicEventDB::instance()->triggerEvent(DynamicEventDB::EVENT_FIRSTAPP_LAUNCHED);
    }

    m_foregroundAppId = appId;
}
