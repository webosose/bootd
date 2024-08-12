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

#include "SurfaceManager.h"

const string SurfaceManager::SERVICE_NAME = "com.webos.surfacemanager";

SurfaceManager::SurfaceManager()
    : AbsService(SERVICE_NAME)
{
}

SurfaceManager::~SurfaceManager()
{
}

bool SurfaceManager::_registerServiceCategory(LSHandle *sh, LSMessage *reply, void *ctx)
{
    SurfaceManagerListener *listener = (SurfaceManagerListener*)ctx;
    Message response(reply);

    if (response.isHubError()) {
        g_Logger.errorLog(Logger::MSGID_CLIENT, "Error occurred : %s", response.getPayload());
        // TODO If this is target service error.
        // subscription should be re-send again
        return false;
    }

    if(!listener->m_call.isActive()) {
        g_Logger.errorLog(Logger::MSGID_CLIENT, "Listener is not activated");
        return false;
    }

    pbnjson::JValue responsePayload = JDomParser::fromString(response.getPayload());
    listener->onSurfaceManagerCategoryChange(responsePayload);
    return true;
}

bool SurfaceManager::registerServiceCategory(Handle *handle, SurfaceManagerListener *listener)
{
    static const char *API = "luna://com.webos.service.bus/signal/registerServiceCategory";
    pbnjson::JValue requestPayload = pbnjson::Object();
    requestPayload.put("serviceName", SurfaceManager::SERVICE_NAME);
    requestPayload.put("category", "/");

    g_Logger.debugLog(Logger::MSGID_CLIENT, "Call %s", API);
    listener->m_call = handle->callMultiReply(
        API,
        requestPayload.stringify().c_str()
    );
    listener->m_call.continueWith(_registerServiceCategory, listener);
    g_Logger.debugLog(Logger::MSGID_CLIENT, "Request : %s", requestPayload.stringify().c_str());
    return true;
}

bool SurfaceManager::showSpinner(Handle *handle, bool show)
{
    static const char *API = "luna://com.webos.surfacemanager/showSpinner";
    pbnjson::JValue requestPayload = pbnjson::Object();
    requestPayload.put("show", show);

    auto call = handle->callOneReply(
        API,
        requestPayload.stringify().c_str()
    );
    g_Logger.debugLog(Logger::MSGID_CLIENT, "Call %s - Request : %s", API, requestPayload.stringify().c_str());
    auto reply = call.get(this->getTimeout());
    if (!reply) {
        g_Logger.errorLog(Logger::MSGID_CLIENT, "No reply in %d ms", this->getTimeout());
        return false;
    }
    if (reply.isHubError()) {
        g_Logger.errorLog(Logger::MSGID_CLIENT, "Error occurred : %s", reply.getPayload());
        return false;
    }
    g_Logger.debugLog(Logger::MSGID_CLIENT, "Response : %s", reply.getPayload());
    return true;
}
