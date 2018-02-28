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

#include "event/StaticEventDB.h"
#include "Main.h"
#include "util/JUtil.h"
#include "util/Logger.h"

#include "ApplicationManager.h"

const string ApplicationManager::SERVICE_NAME = "com.webos.applicationManager";

ApplicationManager::ApplicationManager()
    : AbsService(SERVICE_NAME),
      m_getForegroundAppInfoListener(NULL),
      m_runningListener(NULL)
{
}

ApplicationManager::~ApplicationManager()
{
    if (m_getForegroundAppInfoCall.isActive()) {
        m_getForegroundAppInfoCall.cancel();
    }
    if (m_runningCall.isActive()) {
        m_runningCall.cancel();
    }
}

bool ApplicationManager::_getForegroundAppInfo(LSHandle *sh, LSMessage *reply, void *ctx)
{
    ApplicationManager *applicationManager = (ApplicationManager*)ctx;
    Message response(reply);

    if (response.isHubError()) {
        g_Logger.errorLog(Logger::MSGID_CLIENT, "Error occurred: %s", response.getPayload());
        return false;
    }
    // For readability of bootd log file
    // g_Logger.debugLog(Logger::MSGID_CLIENT, "Response:%s", response.getPayload());

    if(applicationManager->m_getForegroundAppInfoListener == NULL) {
        g_Logger.errorLog(Logger::MSGID_CLIENT, "Listener is null");
        return true;
    }

    pbnjson::JValue responsePayload = JUtil::parse(response.getPayload());
    std::string appId = responsePayload["appId"].asString();
    applicationManager->m_getForegroundAppInfoListener->onForegroundAppChange(appId);
    return true;
}

bool ApplicationManager::_running(LSHandle *sh, LSMessage *reply, void *ctx)
{
    ApplicationManager *applicationManager = (ApplicationManager*)ctx;
    Message response(reply);

    if (response.isHubError()) {
        g_Logger.errorLog(Logger::MSGID_CLIENT, "Error occurred: %s", response.getPayload());
        return false;
    }

    if(applicationManager->m_runningListener == NULL) {
        g_Logger.errorLog(Logger::MSGID_CLIENT, "Listener is null");
        return true;
    }

    pbnjson::JValue responsePayload = JUtil::parse(response.getPayload());
    applicationManager->m_runningListener->onRunning(responsePayload);
    return true;
}

bool ApplicationManager::getForegroundAppInfo(Handle *handle, ApplicationManagerListener *listener)
{
    static const char *API = "luna://com.webos.applicationManager/getForegroundAppInfo";

    string requestPayload = "{ \"subscribe\" : true }";

    if (m_getForegroundAppInfoCall.isActive()) {
        g_Logger.warningLog(Logger::MSGID_CLIENT, "'getForegroundAppInfo' is called already.");
        m_getForegroundAppInfoCall.cancel();
    }
    if (m_getForegroundAppInfoListener != NULL) {
        g_Logger.warningLog(Logger::MSGID_CLIENT, "Listener isn't null");
    }
    m_getForegroundAppInfoListener = listener;

    try {
        g_Logger.debugLog(Logger::MSGID_CLIENT, "Call %s", API);
        m_getForegroundAppInfoCall = handle->callMultiReply(
            API,
            requestPayload.c_str()
        );
        m_getForegroundAppInfoCall.continueWith(_getForegroundAppInfo, this);
    }
    catch (const LS::Error &e) {
        g_Logger.errorLog(Logger::MSGID_CLIENT, "Exception: %s", e.what());
        return false;
    }
    return true;
}

bool ApplicationManager::running(Handle *handle, ApplicationManagerListener *listener)
{
    static const char *API = "luna://com.webos.applicationManager/running";
    pbnjson::JValue requestPayload = pbnjson::Object();
    requestPayload.put("subscribe", true);

    if (m_runningCall.isActive()) {
        g_Logger.warningLog(Logger::MSGID_CLIENT, "'running' is called already.");
        m_runningCall.cancel();
    }
    if (m_runningListener != NULL) {
        g_Logger.warningLog(Logger::MSGID_CLIENT, "Listener isn't null");
    }
    m_runningListener = listener;

    try {
        g_Logger.debugLog(Logger::MSGID_CLIENT, "Call %s", API);
        m_runningCall = handle->callMultiReply(
            API,
            requestPayload.stringify().c_str()
        );
        m_runningCall.continueWith(_running, this);
    }
    catch (const LS::Error &e) {
        g_Logger.errorLog(Logger::MSGID_CLIENT, "Exception: %s", e.what());
        return false;
    }
    return true;
}

bool ApplicationManager::launch(Handle *handle, Application &application, int timeout)
{
    static const char *API = "luna://com.webos.applicationManager/launch";
    // 'boot' are reserved params
    application.getParams().put("boot", true);
    application.printInfo();

    pbnjson::JValue requestPayload = application.getJson();

    try {
        g_Logger.debugLog(Logger::MSGID_CLIENT, "Call %s", API);
        auto call = handle->callOneReply(
            API,
            requestPayload.stringify().c_str()
        );
        g_Logger.debugLog(Logger::MSGID_CLIENT, "Request: %s", requestPayload.stringify().c_str());
        auto reply = call.get(timeout);
        if (!reply) {
            g_Logger.errorLog(Logger::MSGID_CLIENT, "No reply in %d ms", timeout);
            return false;
        }
        if (reply.isHubError()) {
            g_Logger.errorLog(Logger::MSGID_CLIENT, "Error occurred: %s", reply.getPayload());
            return false;
        }

        pbnjson::JValue responsePayload = JUtil::parse(reply.getPayload());
        if (responsePayload["returnValue"].asBool()) {
            g_Logger.debugLog(Logger::MSGID_CLIENT, "Response:launch success(%s)", reply.getPayload());
            application.setLaunched(true);
        } else {
            g_Logger.debugLog(Logger::MSGID_CLIENT, "Response:launch fails(%s)", reply.getPayload());
            application.setLaunched(false);
            return false;
        }
    }
    catch (const LS::Error &e) {
        g_Logger.errorLog(Logger::MSGID_CLIENT, "Exception: %s", e.what());
        return false;
    }
    return true;
}

bool ApplicationManager::closeByAppId(Handle *handle, string appId)
{
    static const char *API = "luna://com.webos.applicationManager/closeByAppId";
    pbnjson::JValue requestPayload = pbnjson::Object();
    requestPayload.put("id", appId);

    try {
        g_Logger.debugLog(Logger::MSGID_CLIENT, "Call %s", API);
        auto call = handle->callOneReply(
            API,
            requestPayload.stringify().c_str()
        );
        g_Logger.debugLog(Logger::MSGID_CLIENT, "Request: %s", requestPayload.stringify().c_str());
        auto reply = call.get(getTimeout());
        if (!reply) {
            g_Logger.errorLog(Logger::MSGID_CLIENT, "No reply in %d ms", getTimeout());
            return false;
        }
        if (reply.isHubError()) {
            g_Logger.errorLog(Logger::MSGID_CLIENT, "Error occurred: %s", reply.getPayload());
            return false;
        }
        g_Logger.debugLog(Logger::MSGID_CLIENT, "Response: %s", reply.getPayload());
    }
    catch (const LS::Error &e) {
        g_Logger.errorLog(Logger::MSGID_CLIENT, "Exception: %s", e.what());
        return false;
    }
    return true;
}

bool ApplicationManager::getAppInfo(Handle *handle, string appId)
{
    static const char *API = "luna://com.webos.applicationManager/getAppInfo";
    pbnjson::JValue requestPayload = pbnjson::Object();
    requestPayload.put("id", appId);

    try {
        g_Logger.debugLog(Logger::MSGID_CLIENT, "Call %s", API);
        auto call = handle->callOneReply(
            API,
            requestPayload.stringify().c_str()
        );
        g_Logger.debugLog(Logger::MSGID_CLIENT, "Request: %s", requestPayload.stringify().c_str());
        auto reply = call.get(getTimeout());
        if (!reply) {
            g_Logger.errorLog(Logger::MSGID_CLIENT, "No reply in %d ms", getTimeout());
            return false;
        }
        if (reply.isHubError()) {
            g_Logger.errorLog(Logger::MSGID_CLIENT, "Error occurred: %s", reply.getPayload());
            return false;
        }

        pbnjson::JValue responsePayload = JUtil::parse(reply.getPayload());
        if (!responsePayload["returnValue"].asBool()) {
            g_Logger.debugLog(Logger::MSGID_CLIENT, "Response : App is not installed (%s)", appId.c_str());
            return false;
        }
        g_Logger.debugLog(Logger::MSGID_CLIENT, "Response : App is installed (%s)", appId.c_str());
    }
    catch (const LS::Error &e) {
        g_Logger.errorLog(Logger::MSGID_CLIENT, "Exception: %s", e.what());
        return false;
    }
    return true;
}

bool ApplicationManager::listLaunchPoints(Handle *handle, int seconds)
{
    static const char *API = "luna://com.webos.applicationManager/listLaunchPoints";
    string requestPayload = "{}";

    try {
        auto call = handle->callOneReply(
            API,
            requestPayload.c_str()
        );
        g_Logger.debugLog(Logger::MSGID_CLIENT, "Call %s - Request: %s", API, requestPayload.c_str());
        auto reply = call.get(seconds*1000);
        if (!reply) {
            g_Logger.errorLog(Logger::MSGID_CLIENT, "No reply in %d ms", seconds*1000);
            return false;
        }
        if (reply.isHubError()) {
            g_Logger.errorLog(Logger::MSGID_CLIENT, "Error occurred: %s", reply.getPayload());
            return false;
        }
        g_Logger.debugLog(Logger::MSGID_CLIENT, "Response: %s", reply.getPayload());
    }
    catch (const LS::Error &e) {
        g_Logger.errorLog(Logger::MSGID_CLIENT, "Exception: %s", e.what());
        return false;
    }
    return true;
}
