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

#include <unistd.h>

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include <boost/bind.hpp>

#include "event/DynamicEventDB.h"
#include "util/JUtil.h"
#include "util/Logger.h"
#include "util/UnixUtils.h"

#include "BootManager.h"

LSSignal BootManager::SIGNAL_TABLE[2] = {
    { NAME_BOOTMANAGER_BOOTTIME, LUNA_SIGNAL_FLAGS_NONE },
    { nullptr }
};

BootManager::BootManager()
    : LS::Handle(LS::registerService(NAME_BOOTMANAGER)),
      m_bootManagerListener(NULL)
{
    m_getBootStatusSubscription.setServiceHandle(this);
}

BootManager::~BootManager()
{
}

void BootManager::init(GMainLoop *mainLoop, BootManagerListener *listener)
{
    m_bootManagerListener = listener;
    attachToLoop(mainLoop);

    LS_CATEGORY_BEGIN(BootManager, "/")
        LS_CATEGORY_METHOD(generateSignal)
        LS_CATEGORY_METHOD(getBootStatus)
    LS_CATEGORY_END

    this->registerCategory(NAME_BOOTMANAGER_SIGNALS, nullptr, SIGNAL_TABLE, nullptr);
}

bool BootManager::generateSignal(LSMessage &message)
{
    Message request(&message);
    pbnjson::JValue responsePayload = pbnjson::Object();
    pbnjson::JValue requestPayload = JUtil::parse(request.getPayload());
    bool returnValue = true;
    string errorText;
    string name;

    g_Logger.debugLog(Logger::MSGID_SERVICE, "Start Handle-generateSignal/Request(%s)", requestPayload.stringify().c_str());

    if (requestPayload.isNull()) {
        errorText = "Invalid JSON argument";
        returnValue = false;
        goto Done;
    }

    if (!requestPayload.hasKey("name") || requestPayload["name"].asString(name) != CONV_OK) {
        errorText = "Error getting 'name' parameter";
        returnValue = false;
        goto Done;
    }

    DynamicEventDB::instance()->triggerEvent(name);

    if (!m_bootManagerListener->onGenerateSignal(requestPayload)) {
        errorText = "onGenerateSignal fails";
        returnValue = false;
    }

Done:
    responsePayload.put("returnValue", returnValue);
    if (!returnValue) {
        responsePayload.put("errorText", errorText);
        g_Logger.warningLog(Logger::MSGID_SERVICE, "Error=Message(%s)", errorText.c_str());
    }
    g_Logger.debugLog(Logger::MSGID_SERVICE, "End Handle-generateSignal:%s", responsePayload.stringify().c_str());
    request.respond(responsePayload.stringify().c_str());
    return true;
}

bool BootManager::getBootStatus(LSMessage &message)
{
    Message request(&message);
    pbnjson::JValue requestPayload = JUtil::parse(request.getPayload());
    pbnjson::JValue bootStatus = m_bootManagerListener->onGetBootStatus();
    bool subscribed = false;

    bootStatus.put("returnValue", true);
    if (request.isSubscription()) {
        m_getBootStatusSubscription.subscribe(request);
        subscribed = true;
    }
    bootStatus.put("subscribed", subscribed);
    request.respond(bootStatus.stringify().c_str());
    g_Logger.debugLog(Logger::MSGID_SERVICE,
                      "Handle-getBootStatus:Sender(%s)/Subscribed(%s)",
                      request.getSenderServiceName(),
                      request.isSubscription() ? "true" : "false");
    return true;
}

void BootManager::postGetBootStatusSubscription(pbnjson::JValue reply)
{
    reply.put("returnValue", true);
    reply.put("subscribed", true);
    if (!m_getBootStatusSubscription.post(reply.stringify().c_str())) {
        g_Logger.errorLog(Logger::MSGID_SERVICE, "Error post getBootStatus");
    }
}

void BootManager::sendSignal(Handle *handle, string name)
{
    static string uri = "luna://" +
                        string(NAME_BOOTMANAGER) + "/" +
                        string(NAME_BOOTMANAGER_SIGNALS) + "/" +
                        string(NAME_BOOTMANAGER_BOOTTIME);
    pbnjson::JValue payload = pbnjson::Object();
    payload.put("returnValue", true);
    payload.put("name", name);
    handle->sendSignal(uri.c_str(), payload.stringify().c_str(), false);
    g_Logger.debugLog(Logger::MSGID_SERVICE, "Signal-%s:%s", uri.c_str(), payload.stringify().c_str());
}
