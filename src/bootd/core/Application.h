// Copyright (c) 2015-2019 LG Electronics, Inc.
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

#ifndef APPLICATION_H_
#define APPLICATION_H_

#include <iostream>

#include "util/JUtil.h"

using namespace std;
using namespace pbnjson;

class Application {
public:
    Application();
    virtual ~Application();
    void clear();

    JValue getJson();
    void printInfo();

    void setAppId(string appId);
    string& getAppId();

    void setParams(pbnjson::JValue params);
    JValue& getParams();

    void setVisible(bool visible);
    bool isVisible();

    void setLaunchSplash(bool launchSplash);
    bool isLaunchSplash();

    void setMvpdApp(bool isMvpd);
    bool isMvpdApp();

    void setLaunched(bool launched);
    bool isLaunched();

    void setForeground(bool foreground);
    bool isForeground();

    void setKeepAlive(bool keepAlive);
    bool isKeepAlive();

    void setDisplayId(int displayId);

private:
    string m_appId;
    int m_displayId;
    JValue m_params;

    bool m_visible;
    bool m_launchSplash;
    bool m_isMvpdApp;
    bool m_isLaunched;
    bool m_isForeground;
    bool m_isKeepAlive;
};


#endif /* APPLICATION_H_ */
