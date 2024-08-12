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

#include "Main.h"
#include "util/JUtil.h"

#include "Configd.h"

const string Configd::SERVICE_NAME = "com.webos.service.config";

Configd::Configd()
    : AbsService(SERVICE_NAME)
{

}

Configd::~Configd()
{

}

bool Configd::getConfigs(Handle *handle, JValue &configNames, JValue &response)
{
    static const char *API = "luna://com.webos.service.config/getConfigs";
    pbnjson::JValue requestPayload = pbnjson::Object();
    requestPayload.put("configNames", configNames);

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

    response = JUtil::parse(reply.getPayload());
    return true;
}

