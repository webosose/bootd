// Copyright (c) 2016-2018 LG Electronics, Inc.
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

#ifndef ABSBOOTSEQUENCER_H_
#define ABSBOOTSEQUENCER_H_

#include <functional>
#include <iostream>

#include <boost/bind.hpp>

#include "core/Application.h"
#include "event/StaticEventDB.h"
#include "event/DynamicEventDB.h"
#include "service/ApplicationManager.h"
#include "service/BootManager.h"

using namespace std;

class AbsBootSequencer : public ApplicationManagerListener, public BootManagerListener, public DynamicEventDBListener {
public:
    AbsBootSequencer();
    virtual ~AbsBootSequencer();

    virtual void start();
    virtual void stop();

    string getBootStatus();
    string getBootTarget();
    string getPowerStatus();

    JValue onGetBootStatus();

    virtual bool onSAMStatusChange(bool isConnected);

    virtual void onForegroundAppChange(string &appId);

protected:
    static int _start(void* ctx);

    static string getBootStatusStr(BootStatus mode);
    static string getBootTargetStr(BootTarget mode);
    static string getPowerStatusStr(PowerStatus mode);
    static PowerStatus getPowerStatusType(string type);

    virtual void doBoot() = 0;

    virtual void proceedCoreBootDone();
    virtual void proceedInitBootDone();
    virtual void proceedDataStoreInitStart();
    virtual void proceedMinimalBootDone();
    virtual void proceedRestBootDone();
    virtual void proceedBootDone();

    bool generateUpstartSignal(const char* signal_name);

    BootStatus m_curBootStatus;
    BootTarget m_curBootTarget;
    PowerStatus m_curPowerStatus;

    Application m_firstApp; // Currently, bootd launches only first app. stay
    BootManager m_bootManager;
    StaticEventDB *m_configuration;

    GMainLoop *m_mainLoop;

    string m_foregroundAppId;
    string m_lastAppId;
};

#endif /* ABSBOOTSEQUENCER_H_ */
