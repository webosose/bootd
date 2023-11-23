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

#include "util/JUtil.h"
#include "util/Logger.h"

#include "AbsService.h"

bool AbsService::getServerStatus(Handle *handle, string serviceName)
{
    static const char *API = "luna://com.webos.service.bus/signal/registerServerStatus";
    pbnjson::JValue requestPayload = pbnjson::Object();
    requestPayload.put("serviceName", serviceName);

    try {
        auto call = handle->callOneReply(
            API,
            requestPayload.stringify().c_str()
        );
        g_Logger.debugLog(Logger::MSGID_CLIENT, "Call %s - Request : %s", API, requestPayload.stringify().c_str());
        auto reply = call.get(AbsService::TIMEOUT_MAX);
        if (!reply) {
            g_Logger.errorLog(Logger::MSGID_CLIENT, "No reply in %d ms", AbsService::TIMEOUT_MAX);
            return false;
        }
        if (reply.isHubError()) {
            g_Logger.errorLog(Logger::MSGID_CLIENT, "Error occurred : %s", reply.getPayload());
            return false;
        }

        g_Logger.debugLog(Logger::MSGID_CLIENT, "Response : %s", reply.getPayload());
        pbnjson::JValue responsePayload = JUtil::parse(reply.getPayload());
        if (responsePayload.hasKey("connected") && responsePayload["connected"].asBool()) {
            return true;
        }
    }
    catch (const LS::Error &e) {
        g_Logger.errorLog(Logger::MSGID_CLIENT, "Exception : %s", e.what());
        return false;
    }
    return false;
}

AbsService::AbsService(string name)
    : m_name(std::move(name)),
      m_serverStatus()
{
}

AbsService::~AbsService()
{
    try {
        if (m_serverStatus) {
            m_serverStatus.cancel();
        }
    }
    catch (const LS::Error &e) {
        g_Logger.errorLog(Logger::MSGID_CLIENT, "Exception : %s", e.what());
    }
}

string& AbsService::getName()
{
    return m_name;
}

int AbsService::getTimeout()
{
    return TIMEOUT_MAX;
}

bool AbsService::registerServerStatus(Handle *handle, ServerStatusCallback callback)
{
    g_Logger.debugLog(Logger::MSGID_CLIENT, "registerServerStatus : %s", m_name.c_str());
    if (m_serverStatus) {
        m_serverStatus.cancel();
    }
    // TODO: Do we need to consider service on/off status in service classes?
    // If it is 'yes', we need to implement some logic for that
    // For example, subscriptions should be closed if service is down.
    m_callback = std::move(callback);
    m_serverStatus = handle->registerServerStatus(m_name.c_str(), m_callback);
    return true;
}
