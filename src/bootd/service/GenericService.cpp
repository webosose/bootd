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

#include "GenericService.h"

const string GenericService::SERVICE_NAME = "com.webos.genericservice";

GenericService::GenericService()
    : AbsService(SERVICE_NAME)
{
}

GenericService::~GenericService()
{
}

bool GenericService::_callMultiReply(LSHandle *sh, LSMessage *reply, void *ctx)
{
    GenericServiceListener *listener = (GenericServiceListener*)ctx;
    Message response(reply);

    if (response.isHubError()) {
        g_Logger.errorLog(Logger::MSGID_CLIENT, "Error occurred : %s", response.getPayload());
        // TODO If this is target service error.
        // subscription should be re-send again
        return false;
    }

    if(!listener->m_call.isActive()) {
        g_Logger.errorLog(Logger::MSGID_CLIENT, "Listener is not activated");
        return true;
    }

    pbnjson::JValue responsePayload = JDomParser::fromString(response.getPayload());
    listener->onSubscription(responsePayload);
    return true;
}
