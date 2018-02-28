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

#ifndef SETTINGSSERVICE_H_
#define SETTINGSSERVICE_H_

#include <iostream>
#include <map>

#include <luna-service2++/call.hpp>
#include <luna-service2++/handle.hpp>
#include <pbnjson.hpp>

#include "AbsService.h"

using namespace std;
using namespace LS;
using namespace pbnjson;

class SettingsService : public AbsService {
public:
    static SettingsService* instance()
    {
        static SettingsService _instance;
        return &_instance;
    }

    virtual ~SettingsService();

    bool getSystemSettings(Handle *handle, string category, JValue keys, JValue &result);

    static const char *KEY_ENABLE_HOTEL_MODE;
    static const char *KEY_ENABLE_AUTO_BROWSER;
    static const char *KEY_ENABLE_FULL_SCREEN;
    static const char *KEY_AUTO_BROWSER_URL;
private:
    SettingsService();

    static const string SERVICE_NAME;
};

#endif /* SETTINGSSERVICE_H_ */
