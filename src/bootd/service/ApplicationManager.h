// Copyright (c) 2015-2018 LG Electronics, Inc.
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

#ifndef APPLICATIONMANAGER_H_
#define APPLICATIONMANAGER_H_

#include <iostream>

#include <luna-service2++/call.hpp>
#include <luna-service2++/handle.hpp>
#include <pbnjson.hpp>

#include "core/Application.h"

#include "AbsService.h"

using namespace std;
using namespace LS;

class ApplicationManagerListener {
public:
    ApplicationManagerListener() {};
    virtual ~ApplicationManagerListener() {};

    virtual void onForegroundAppChange(string &appId) {};
    virtual void onRunning(JValue &runninglist) {};
};

class ApplicationManager : public AbsService {
public:
    static ApplicationManager* instance()
    {
        static ApplicationManager _instance;
        return &_instance;
    }

    virtual ~ApplicationManager();

    bool getForegroundAppInfo(Handle *handle, ApplicationManagerListener *listener);
    bool running(Handle *handle, ApplicationManagerListener *listener);
    bool launch(Handle *handle, Application &application, int timeout = TIMEOUT_MAX);
    bool closeByAppId(Handle *handle, string appId);
    bool getAppInfo(Handle *handle, string appId);
    bool listLaunchPoints(Handle *handle, int seconds);

private:
    static bool _getForegroundAppInfo(LSHandle *sh, LSMessage *reply, void *ctx);
    static bool _running(LSHandle *sh, LSMessage *reply, void *ctx);

    ApplicationManager();

    static const string SERVICE_NAME;

    ApplicationManagerListener *m_getForegroundAppInfoListener;
    Call m_getForegroundAppInfoCall;

    ApplicationManagerListener *m_runningListener;
    Call m_runningCall;
};

#endif /* APPLICATIONMANAGER_H_ */
