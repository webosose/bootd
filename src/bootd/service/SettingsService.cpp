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

#include "event/StaticEventDB.h"
#include "util/JUtil.h"
#include "util/Logger.h"

#include "SettingsService.h"

const char *SettingsService::KEY_AUTO_BROWSER_URL = "autoBrowserUrl";
const char *SettingsService::KEY_ENABLE_HOTEL_MODE = "enableHotelMode";
const char *SettingsService::KEY_ENABLE_FULL_SCREEN = "enableFullscreen";
const char *SettingsService::KEY_ENABLE_AUTO_BROWSER = "enableAutoBrowser";

const string SettingsService::SERVICE_NAME = "com.webos.settingsservice";

SettingsService::SettingsService()
    : AbsService(SERVICE_NAME)
{
}

SettingsService::~SettingsService()
{
}

bool SettingsService::getSystemSettings(Handle *handle, string category, JValue keys, JValue &result)
{
    static const char *API = "luna://com.webos.settingsservice/getSystemSettings";
    // clear and set default value
    pbnjson::JValue requestPayload = pbnjson::Object();
    requestPayload.put("category", category);
    requestPayload.put("keys", keys);

    try {
        g_Logger.debugLog(Logger::MSGID_CLIENT, "Call %s", API);
        auto call = handle->callOneReply(
            API,
            requestPayload.stringify().c_str()
        );
        g_Logger.debugLog(Logger::MSGID_CLIENT, "%s: Request:%s", API, requestPayload.stringify().c_str());
        auto reply = call.get(getTimeout());
        if (!reply) {
            g_Logger.errorLog(Logger::MSGID_CLIENT, "No reply in %d ms", getTimeout());
            return false;
        }
        if (reply.isHubError()) {
            g_Logger.errorLog(Logger::MSGID_CLIENT, "Error occurred: %s", reply.getPayload());
            return false;
        }
        g_Logger.debugLog(Logger::MSGID_CLIENT, "Response:%s", reply.getPayload());
        pbnjson::JValue responsePayload = JUtil::parse(reply.getPayload());

        if (!responsePayload.hasKey("settings")) {
            return false;
        }

        for (auto key : keys.items()) {
            if (!responsePayload["settings"].hasKey(key.asString())) {
                return false;
            }
        }

        result = responsePayload["settings"];
    }
    catch (const LS::Error &e) {
        g_Logger.errorLog(Logger::MSGID_CLIENT, "Exception: %s", e.what());
        return false;
    }
    return true;
}
