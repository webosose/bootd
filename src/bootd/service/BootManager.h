// Copyright (c) 2013-2018 LG Electronics, Inc.
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

#ifndef BOOTMANAGER_H
#define BOOTMANAGER_H

#ifndef TEST_BOOTDSERVICE
#define TEST_BOOTDSERVICE
#endif

#include <stdbool.h>

#include <functional>
#include <list>
#include <map>
#include <set>
#include <string>

#include <boost/function.hpp>
#include <gio/gio.h>
#include <glib.h>
#include <luna-service2/lunaservice.hpp>
#include <pbnjson.hpp>

#include "core/SignalManager.h"
#include "core/Application.h"
#include "Main.h"

using namespace std;
using namespace LS;
using namespace std::placeholders;

class BootManagerListener {
public:
    BootManagerListener() {};
    virtual ~BootManagerListener() {};

    virtual bool onGenerateSignal(JValue &requestPayload) { return true; };
    virtual JValue onGetBootStatus() = 0;
};

class BootManager : public Handle {
public:
    BootManager();
    virtual ~BootManager();

    void init(GMainLoop *mainLoop, BootManagerListener *listener);
    void postGetBootStatusSubscription(pbnjson::JValue reply);
    void sendSignal(Handle *handle, string name);

private:
    static LSSignal SIGNAL_TABLE[2];

    // BootManager APIs
    bool generateSignal(LSMessage &message);
    bool getBootStatus(LSMessage &message);

    SubscriptionPoint m_getBootStatusSubscription;
    BootManagerListener *m_bootManagerListener;
};

#endif // BOOTMANAGER_H
