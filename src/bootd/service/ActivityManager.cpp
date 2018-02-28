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

#include <pbnjson.hpp>

#include "core/Activity.h"

#include "ActivityManager.h"

const string ActivityManager::SERVICE_NAME = "com.webos.service.activitymanager";

ActivityManager::ActivityManager()
    : AbsService(SERVICE_NAME)
{
}

ActivityManager::~ActivityManager()
{
}

bool ActivityManager::restart(Handle *handle, Activity &activity)
{
    // TODO: need to verify, this function is not used
    static const char *API = "luna://com.webos.service.activitymanager/complete";
    pbnjson::JValue requestPayload = pbnjson::Object();
    requestPayload.put("restart", true);

    try {
        auto call = handle->callOneReply(
            API,
            requestPayload.stringify().c_str()
        );
        g_Logger.debugLog(Logger::MSGID_CLIENT, "Call %s - Request: %s", API, requestPayload.stringify().c_str());
        auto reply = call.get(this->getTimeout());
        if (!reply) {
            g_Logger.errorLog(Logger::MSGID_CLIENT, "No reply in %d ms", this->getTimeout());
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

bool ActivityManager::create(Handle *handle, Activity &activity)
{
    static const char *API = "luna://com.webos.service.activitymanager/create";
    pbnjson::JValue requestPayload = pbnjson::Object();
    requestPayload.put("activity", activity.getObject());
    requestPayload.put("replace", true);
    requestPayload.put("start", true);

    try {
        auto call = handle->callOneReply(
            API,
            requestPayload.stringify().c_str()
        );
        g_Logger.debugLog(Logger::MSGID_CLIENT, "Call %s - Request: %s", API, requestPayload.stringify().c_str());
        auto reply = call.get(this->getTimeout());
        if (!reply) {
            g_Logger.errorLog(Logger::MSGID_CLIENT, "No reply in %d ms", this->getTimeout());
            return false;
        }
        if (reply.isHubError()) {
            g_Logger.errorLog(Logger::MSGID_CLIENT, "Error occurred: %s", reply.getPayload());
            return false;
        }
        pbnjson::JValue replyPayload = JDomParser::fromString(reply.getPayload());
        int activityId = replyPayload["activityId"].asNumber<int32_t>();
        activity.setId(activityId);
        g_Logger.debugLog(Logger::MSGID_CLIENT, "Response: %s, activityName: %s, activityId: %d",
                reply.getPayload(),
                activity.getName().c_str(),
                activityId);
    }
    catch (const LS::Error &e) {
        g_Logger.errorLog(Logger::MSGID_CLIENT, "Exception: %s", e.what());
        return false;
    }
    return true;
}
