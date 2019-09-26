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

#include "event/DynamicEventDB.h"
#include "service/ApplicationManager.h"
#include "util/Logger.h"

#include "DefaultBootSequencer.h"

DefaultBootSequencer::DefaultBootSequencer()
    : AbsBootSequencer()
{
}

DefaultBootSequencer::~DefaultBootSequencer()
{
}

void DefaultBootSequencer::doBoot()
{
    /* DefaultBootSequencer is just booting. */
    PmtScopedBlock(PMTRACE_DEFAULT_CATEGORY);
    g_Logger.debugLog(Logger::MSGID_BOOTSEQUENCER, "Start DefaultBootSequencer");

    m_bootManager.init(m_mainLoop, this);

    ApplicationManager::instance()->registerServerStatus(&m_bootManager,
            std::bind(&AbsBootSequencer::onSAMStatusChange, this, std::placeholders::_1));

    m_curBootStatus = BootStatus::BootStatus_normal;
    m_curPowerStatus = PowerStatus::PowerStatus_active;
    m_curBootTarget = BootTarget::BootTarget_hardware;

    // Always launch firstapp (bareapp) first
    launchTargetApp("bareapp", true, false);
    DynamicEventDB::instance()->waitEvent(m_mainLoop, DynamicEventDB::EVENT_FIRSTAPP_LAUNCHED, EventCoreTimeout::EventCoreTimeout_Middle);

    proceedCoreBootDone();
    proceedInitBootDone();
    proceedDataStoreInitStart();
    ApplicationManager::instance()->listLaunchPoints(&m_bootManager, EventCoreTimeout::EventCoreTimeout_Max);
    proceedMinimalBootDone();
    proceedRestBootDone();
    proceedBootDone();

    ApplicationManager::instance()->running(&m_bootManager, this);

    DynamicEventDB::instance()->triggerEvent(DynamicEventDB::EVENT_BOOT_COMPLETE);
    g_Logger.infoLog(Logger::MSGID_BOOTSEQUENCER, "Bootd's job is done");
}

void DefaultBootSequencer::launchTargetApp(string appId, bool visible, bool keepAlive)
{
    Application application;
    application.setAppId(appId);
    application.setVisible(visible);

    if (keepAlive)
        application.setKeepAlive(keepAlive);

    for (int i = 0; i < COUNT_LAUNCH_RETRY; i++) {
        if (ApplicationManager::instance()->launch(&m_bootManager, application)) {
            if (visible)
                g_Logger.infoLog(Logger::MSGID_BOOTSEQUENCER, "Launch target app (%s) on foreground", application.getAppId().c_str());
            else
                g_Logger.infoLog(Logger::MSGID_BOOTSEQUENCER, "Launch target app (%s) on background", application.getAppId().c_str());
            break;
        }
        g_Logger.warningLog(Logger::MSGID_BOOTSEQUENCER, "Fail to launch '%s'. Retry...(%d)", application.getAppId().c_str(), i);
    }
}

void DefaultBootSequencer::onRunning(JValue &runninglist)
{
    bool isRunningHomeApp = false;
    bool isRunningVolumeApp = false;

    g_Logger.debugLog(Logger::MSGID_BOOTSEQUENCER, "Running list : %s", runninglist.stringify().c_str());

    for (int i = 0; i < runninglist["running"].arraySize(); i++) {
        if (runninglist["running"][i]["id"].asString() == "com.webos.app.home")
            isRunningHomeApp = true;
        else if (runninglist["running"][i]["id"].asString() == "com.webos.app.volume")
            isRunningVolumeApp = true;
    }

    if (!isRunningHomeApp)
        launchTargetApp("com.webos.app.home", false, true);
    if (!isRunningVolumeApp)
        launchTargetApp("com.webos.app.volume", false, true);
}
