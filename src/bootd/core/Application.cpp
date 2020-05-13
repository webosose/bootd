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

#include "util/Logger.h"

#include "Application.h"

Application::Application()
{
    clear();
}

Application::~Application()
{
}

void Application::clear()
{
    m_appId = "";
    m_displayId = 0;
    // Basically, all apps launched by bootd is 'visible':true and 'splash':false
    m_visible = true;
    m_launchSplash = false;
    m_isMvpdApp = false;
    m_isLaunched = false;
    m_isForeground = false;
    m_isKeepAlive = false;
    m_params = pbnjson::Object();
}

JValue Application::getJson()
{
    JValue object = pbnjson::Object();
    object.put("id", m_appId);
    // visible(launchedHidden) should be used in params
    m_params.put("launchedHidden", !m_visible);
    if (m_isKeepAlive) {
        object.put("keepAlive", m_isKeepAlive);
    }
    object.put("noSplash", !m_launchSplash);
    if (!m_params.isNull()) {
        object.put("params", m_params);
    }

    return object;
}

void Application::printInfo()
{
    g_Logger.debugLog(Logger::MSGID_GENERAL,
                      "AppInfo=appId(%s), Visible(%s), LaunchSplash(%s), MVPD(%s), Launched(%s), KeepAlive(%s)",
                      m_appId.c_str(),
                      isVisible() ? "true" : "false",
                      isLaunchSplash() ? "true" : "false",
                      isMvpdApp() ? "true" : "false",
                      isLaunched() ? "true" : "false",
                      isKeepAlive() ? "true" : "false");
}

void Application::setAppId(string appId)
{
    m_appId = appId;
}

string& Application::getAppId()
{
    return m_appId;
}

void Application::setParams(JValue params) {
    if (params.isNull()) {
        g_Logger.warningLog(Logger::MSGID_GENERAL, "Param is null. Invalid");
        return;
    }
    g_Logger.debugLog(Logger::MSGID_GENERAL,
                      "Params is not null: from(%s)->to(%s)",
                      m_params.stringify().c_str(),
                      params.stringify().c_str());
    m_params = params;
}

JValue& Application::getParams()
{
    return m_params;
}

void Application::setVisible(bool visible)
{
    m_visible = visible;
}

bool Application::isVisible()
{
    return m_visible;
}

void Application::setLaunchSplash(bool launchSplash)
{
    m_launchSplash = launchSplash;
}

bool Application::isLaunchSplash()
{
    return m_launchSplash;
}

void Application::setMvpdApp(bool isMvpd)
{
    m_isMvpdApp = isMvpd;
}

bool Application::isMvpdApp()
{
    return m_isMvpdApp;
}

void Application::setLaunched(bool launched)
{
    m_isLaunched = launched;
}

bool Application::isLaunched()
{
    return m_isLaunched;
}

void Application::setForeground(bool foreground)
{
    m_isForeground = foreground;
}

bool Application::isForeground()
{
    return m_isForeground;
}

void Application::setKeepAlive(bool keepAlive)
{
    m_isKeepAlive = keepAlive;
}

bool Application::isKeepAlive()
{
    return m_isKeepAlive;
}

void Application::setDisplayId(int displayId)
{
    m_displayId = displayId;
    m_params.put("displayAffinity", m_displayId);
}
