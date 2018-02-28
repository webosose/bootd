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

#ifndef SIGNALMANAGER_H_
#define SIGNALMANAGER_H_

#include <stdlib.h>

#include <iostream>
#include <map>

#include <glib.h>

#include "util/JUtil.h"

using namespace std;
using namespace pbnjson;

class SignalManager {
public:
    // oneshot Signals
    static const char *CORE_BOOT_DONE;
    static const char *INIT_BOOT_DONE;
    static const char *DATASTORE_INIT_START;
    static const char *MINIMAL_BOOT_DONE;
    static const char *REST_BOOT_DONE;
    static const char *BOOT_DONE;

    static const char *BOOTMODE_NORMAL_INIT_BOOT_DONE;
    static const char *BOOTMODE_NORMAL_BOOT_DONE;

    static SignalManager* instance()
    {
        static SignalManager _instance;
        return &_instance;
    }

    virtual ~SignalManager();

    bool isGenerated(string name);
    bool generate(string name, const bool isSync = false);
    bool startBootMode(string mode, const bool isSync = false);
    JValue getSignals();

private:
    SignalManager();
    map<string, bool> m_oneshotSignals;
    vector<string> m_periodicSignals;

};

#endif /* SIGNALMANAGER_H_ */
