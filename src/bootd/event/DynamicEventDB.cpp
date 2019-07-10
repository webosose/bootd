// Copyright (c) 2013-2019 LG Electronics, Inc.
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

#include "DynamicEventDB.h"

const char *DynamicEventDB::EVENT_BOOT_COMPLETE = "boot-complete";
const char *DynamicEventDB::EVENT_FIRSTAPP_LAUNCHED = "firstapp-launched";

const char *DynamicEventDB::KEY_READY = "ready";
const char *DynamicEventDB::KEY_TIME = "time";
const char *DynamicEventDB::KEY_EXTRA = "extra";

DynamicEventDB::DynamicEventDB()
    : m_isInLoop(false),
      m_waitTimeoutId(0),
      m_waitEventTimeoutId(0),
      m_waitFileTimeoutId(0),
      m_dir(NULL),
      m_dirMonitor(NULL),
      m_dirMonitorId(0)
{
    m_eventDatabase = pbnjson::Object();
}

DynamicEventDB::~DynamicEventDB()
{
}

bool DynamicEventDB::isWaitSomething()
{
    return m_isInLoop;
}

bool DynamicEventDB::hasEvent(string eventName)
{
    return m_eventDatabase.hasKey(eventName);
}

bool DynamicEventDB::getEventStatus(string eventName)
{
    if (!m_eventDatabase.hasKey(eventName)) {
        g_Logger.debugLog(Logger::MSGID_EVENT,
                          "No event status : '%s'",
                          eventName.c_str());
        return false;
    }
    return m_eventDatabase[eventName][KEY_READY].asBool();
}

bool DynamicEventDB::getEventStatus(string eventName, JValue &extra)
{
    if (!getEventStatus(eventName))
        return false;

    extra = m_eventDatabase[eventName][KEY_EXTRA].duplicate();
    return true;
}

bool DynamicEventDB::replaceEventsInfo(JValue &events, string origin, string &result)
{
    result = origin;

    for (int i = 0; i < events.arraySize(); i++) {
        string pattern = "${" + events[i].asString() + "}";
        string::size_type pos = result.find(pattern);

        if (pos != string::npos) {
            JValue extra;
            if (!getEventStatus(events[i].asString(), extra))
                return false;

            result.replace(pos, pattern.size(), extra.asString());
        }
    }
    return true;
}

bool DynamicEventDB::removeEvent(string eventName)
{
    if (!m_eventDatabase.hasKey(eventName)) {
        g_Logger.debugLog(Logger::MSGID_EVENT,
                          "'%s' event is not database",
                          eventName.c_str());
        return true;
   }

   return m_eventDatabase.remove(eventName);
}

int DynamicEventDB::_waitTimeout(void* ctx)
{
    g_Logger.debugLog(Logger::MSGID_EVENT, "Timeout is done");
    DynamicEventDB *eventCore = (DynamicEventDB*)ctx;
    eventCore->m_waitTimeoutId = 0;

    if (!eventCore->m_isInLoop) {
        g_Logger.warningLog(Logger::MSGID_EVENT, "Timeout is done without loop. This might be a bug");
    } else {
        eventCore->m_isInLoop = false;
    }
    return G_SOURCE_REMOVE;
}

bool DynamicEventDB::waitTimeout(GMainLoop* mainLoop, int seconds)
{
    if (m_isInLoop) {
        g_Logger.warningLog(Logger::MSGID_EVENT, "EventCore already waits something. This might be a bug");
        return false;
    }
    g_Logger.debugLog(Logger::MSGID_EVENT,
                      "EventCore waits timeout(%d)",
                      seconds);
    m_waitTimeoutId = g_timeout_add_seconds(seconds, DynamicEventDB::_waitTimeout, this);
    m_isInLoop = true;
    while (m_isInLoop) {
        g_main_context_iteration(g_main_loop_get_context(mainLoop), TRUE);
    }
    return true;
}

int DynamicEventDB::_waitEventTimeout(void* ctx)
{
    DynamicEventDB *eventCore = (DynamicEventDB*)ctx;
    g_Logger.debugLog(Logger::MSGID_EVENT, "Event (%s) timeout is done", eventCore->m_waitEvent.c_str());
    eventCore->m_waitEventTimeoutId = 0;

    if (!eventCore->m_isInLoop) {
        g_Logger.warningLog(Logger::MSGID_EVENT, "Timeout is done without loop. This might be a bug");
    } else {
        eventCore->m_isInLoop = false;
    }
    return G_SOURCE_REMOVE;
}

bool DynamicEventDB::existEvent(GMainLoop* mainLoop, string eventName, int seconds)
{
    if (m_isInLoop) {
        g_Logger.warningLog(Logger::MSGID_EVENT, "EventCore already waits something. This might be a bug");
        return false;
    }

    if (m_eventDatabase.hasKey(eventName)) {
        g_Logger.debugLog(Logger::MSGID_EVENT,
                          "'%s' event is already occured",
                          eventName.c_str());
        return true;
    }

    g_Logger.debugLog(Logger::MSGID_EVENT,
                      "EventCore waits event(%s)/timeout(%d)",
                      eventName.c_str(), seconds);

    m_existEvent = eventName;
    m_waitEventTimeoutId = g_timeout_add_seconds(seconds, DynamicEventDB::_waitEventTimeout, this);
    m_isInLoop = true;
    while (m_isInLoop) {
        g_main_context_iteration(g_main_loop_get_context(mainLoop), TRUE);
    }
    return true;
}

bool DynamicEventDB::waitEvent(GMainLoop* mainLoop, string eventName, int seconds)
{
    if (m_isInLoop) {
        g_Logger.warningLog(Logger::MSGID_EVENT, "EventCore already waits something. This might be a bug");
        return false;
    }

    if (m_eventDatabase.hasKey(eventName) && m_eventDatabase[eventName][KEY_READY].asBool()) {
        g_Logger.debugLog(Logger::MSGID_EVENT,
                          "'%s' event is already completed",
                          eventName.c_str());
        return true;
    }

    g_Logger.debugLog(Logger::MSGID_EVENT,
                      "EventCore waits event(%s)/timeout(%d)",
                      eventName.c_str(), seconds);

    m_waitEvent = eventName;
    m_waitEventTimeoutId = g_timeout_add_seconds(seconds, DynamicEventDB::_waitEventTimeout, this);
    m_isInLoop = true;
    while (m_isInLoop) {
        g_main_context_iteration(g_main_loop_get_context(mainLoop), TRUE);
    }
    return true;
}

int DynamicEventDB::_waitAsyncEventTimeout(void* ctx)
{
    AsyncEventData* asyncEvent = (AsyncEventData*)ctx;
    g_Logger.debugLog(Logger::MSGID_EVENT,
                      "Async event (%s) timeout is done",
                      asyncEvent->m_eventName.c_str());

    DynamicEventDB::instance()->callAsyncEventListeners(asyncEvent->m_eventName);
    return G_SOURCE_REMOVE;
}

bool DynamicEventDB::waitAsyncEvent(GMainLoop* mainLoop, string eventName, int seconds, DynamicEventDBListener *listener)
{
    if (m_eventDatabase.hasKey(eventName) && m_eventDatabase[eventName][KEY_READY].asBool()) {
        g_Logger.debugLog(Logger::MSGID_EVENT,
                          "'%s' event is already completed",
                          eventName.c_str());
        listener->onWaitAsyncEvent(eventName);
        return true;
    }

    AsyncEventData* asyncEvent = new AsyncEventData();
    guint waitAsyncEventTimeoutId = g_timeout_add_seconds(seconds, DynamicEventDB::_waitAsyncEventTimeout, asyncEvent);
    asyncEvent->m_eventName = eventName;
    asyncEvent->m_listener = listener;
    asyncEvent->m_timeoutId = waitAsyncEventTimeoutId;

    g_Logger.debugLog(Logger::MSGID_EVENT,
                      "EventCore waits async event(%s)/timeout(%d)/id(%u)",
                      eventName.c_str(), seconds, waitAsyncEventTimeoutId);

    map<string, vector<AsyncEventData*>>::iterator asyncEventIter = m_asyncEventData.find(eventName);
    if (asyncEventIter != m_asyncEventData.end()) {
        asyncEventIter->second.push_back(asyncEvent);
    } else {
        vector<AsyncEventData*> asyncEventVector;
        asyncEventVector.push_back(asyncEvent);
        m_asyncEventData.insert(pair<string, vector<AsyncEventData*>> (eventName, asyncEventVector));
    }

    return true;
}

bool DynamicEventDB::triggerEvent(string eventName, JValue extra)
{
    if (!m_eventDatabase.hasKey(eventName)) {
        pbnjson::JValue event = pbnjson::Object();
        event.put(KEY_READY, true);
        event.put(KEY_EXTRA, extra);
        m_eventDatabase.put(eventName, event);
        g_Logger.debugLog(Logger::MSGID_EVENT,
                          "Add new event : '%s' [Status : trigger] %s",
                          eventName.c_str(),
                          extra.stringify().c_str());
        goto DONE;
    }

    m_eventDatabase[eventName].put(KEY_READY, true);
    m_eventDatabase[eventName].put(KEY_EXTRA, extra);

DONE:
    if ((m_waitEvent == eventName || m_existEvent == eventName) && m_waitEventTimeoutId > 0) {
        g_source_remove(m_waitEventTimeoutId);
        m_waitEventTimeoutId = 0;
        m_isInLoop = false;
    }

    g_Logger.debugLog(Logger::MSGID_EVENT,
                      "Trigger '%s' event (%s)",
                      eventName.c_str(),
                      extra.stringify().c_str());

    callEventListeners(eventName, TriggerEvent);

    if (!m_asyncEventData.empty())
        callAsyncEventListeners(eventName);

    return true;
}

bool DynamicEventDB::clearEvent(string eventName)
{
    if (!m_eventDatabase.hasKey(eventName)) {
        pbnjson::JValue event = pbnjson::Object();
        event.put(KEY_READY, false);
        event.put(KEY_EXTRA, "");
        m_eventDatabase.put(eventName, event);
        g_Logger.debugLog(Logger::MSGID_EVENT,
                          "Add new event : '%s', [Status : clear]",
                          eventName.c_str());
        goto DONE;
    }

    m_eventDatabase[eventName].put(KEY_READY, false);
    m_eventDatabase[eventName].put(KEY_EXTRA, pbnjson::Object());

DONE:
    if (m_existEvent == eventName && m_waitEventTimeoutId > 0) {
        g_source_remove(m_waitEventTimeoutId);
        m_waitEventTimeoutId = 0;
        m_isInLoop = false;
    }
    callEventListeners(eventName, ClearEvent);
    return true;
}

void DynamicEventDB::callEventListeners(string eventName, EventHandleType type)
{
    struct timespec curTime;
    char timeBuffer[256] = { 0, };
    g_Logger.getCurrentTime(curTime);
    sprintf(timeBuffer, "%ld.%09ld", curTime.tv_sec, curTime.tv_nsec);
    m_eventDatabase[eventName].put(KEY_TIME, timeBuffer);
}

void DynamicEventDB::callAsyncEventListeners(string eventName)
{
    map<string, vector<AsyncEventData*>>::iterator asyncEventIter = m_asyncEventData.find(eventName);
    if (asyncEventIter == m_asyncEventData.end()) {
        g_Logger.debugLog(Logger::MSGID_EVENT, "No wait async event (%s)", eventName.c_str());
        return;
    }

    for (vector<AsyncEventData*>::iterator iter = asyncEventIter->second.begin(); iter != asyncEventIter->second.end(); ) {
        (*iter)->m_listener->onWaitAsyncEvent(eventName);
        g_Logger.debugLog(Logger::MSGID_EVENT, "Remove async event timeout resource (%u)", (*iter)->m_timeoutId);
        g_source_remove((*iter)->m_timeoutId);
        delete *iter;
        iter = asyncEventIter->second.erase(iter);
    }

    m_asyncEventData.erase(asyncEventIter);
}

int DynamicEventDB::_waitFileTimeout(void* ctx)
{
    g_Logger.debugLog(Logger::MSGID_EVENT, "File timeout is done");
    DynamicEventDB *eventCore = (DynamicEventDB*)ctx;
    eventCore->m_waitFileTimeoutId = 0;

    eventCore->cleanFileMonitor();
    if (!eventCore->m_isInLoop) {
        g_Logger.warningLog(Logger::MSGID_EVENT, "Timeout is done without loop. This might be a bug");
    } else {
        eventCore->m_isInLoop = false;
    }
    return G_SOURCE_REMOVE;
}

void DynamicEventDB::_waitFile(GFileMonitor *monitor,
                          GFile *file,
                          GFile *other_file,
                          GFileMonitorEvent event_type,
                          gpointer user_data)
{
    DynamicEventDB *eventCore = (DynamicEventDB *)user_data;
    if (G_FILE_MONITOR_EVENT_CREATED != event_type) {
        g_Logger.debugLog(Logger::MSGID_EVENT, "Not creation event. Keep waiting...");
        return;
    }

    // UTP service is watching initfile as well. Don't remove file.
    // if (!g_file_delete(file, NULL, NULL)) {
    //    g_Logger.errorLog(Logger::MSGID_EVENT, "Deleting file fails");
    //}

    if (eventCore->m_waitFileTimeoutId > 0) {
        g_source_remove(eventCore->m_waitFileTimeoutId);
        eventCore->m_waitFileTimeoutId = 0;
    }
    eventCore->m_isInLoop = false;
    g_Logger.debugLog(Logger::MSGID_EVENT, "EventCore exits because file is created");
    eventCore->cleanFileMonitor();
}

bool DynamicEventDB::waitFile(GMainLoop* mainLoop, string filePath, int seconds)
{
    if (m_isInLoop) {
        g_Logger.warningLog(Logger::MSGID_EVENT, "EventCore already waits something. This might be a bug");
        return false;
    }

    if (access(filePath.c_str(), F_OK) == 0) {
        g_Logger.debugLog(Logger::MSGID_EVENT, "'%s' file is already created", filePath.c_str());
        // UTP service is watching initfile as well. Don't remove file.
        // unlink(filePath.c_str());
        return true;
    }

    m_dir = g_file_new_for_path(filePath.c_str());
    m_dirMonitor = g_file_monitor_file(m_dir, G_FILE_MONITOR_NONE, NULL, NULL);
    if (m_dirMonitor == NULL) {
        g_object_unref(m_dir);
        g_Logger.warningLog(Logger::MSGID_EVENT, "Creating monitor fails.");
        return false;
    }

    g_Logger.debugLog(Logger::MSGID_EVENT, "EventCore waits file(%s)/timeout(%d)", filePath.c_str(), seconds);
    m_dirMonitorId = g_signal_connect(m_dirMonitor, "changed", G_CALLBACK(_waitFile), this);
    m_waitFileTimeoutId = g_timeout_add_seconds(seconds, &DynamicEventDB::_waitFileTimeout, this);
    m_isInLoop = true;
    while (m_isInLoop) {
        g_main_context_iteration(g_main_loop_get_context(mainLoop), TRUE);
    }
    return true;
}

void DynamicEventDB::cleanFileMonitor()
{
    if (0 != m_dirMonitorId)
        g_signal_handler_disconnect(m_dirMonitor, m_dirMonitorId);
    if (NULL != m_dirMonitor)
        g_object_unref(m_dirMonitor);
    if (NULL != m_dir)
        g_object_unref(m_dir);
}

void DynamicEventDB::printAsyncEventMap()
{
    g_Logger.debugLog(Logger::MSGID_EVENT, "Information of async event map");
    if (m_asyncEventData.empty()) {
        g_Logger.debugLog(Logger::MSGID_EVENT, "No data");
        return;
    }

    map<string, vector<AsyncEventData*>>::iterator iter;
    for(iter = m_asyncEventData.begin(); iter != m_asyncEventData.end(); iter++) {
        g_Logger.debugLog(Logger::MSGID_EVENT, "\tEvent : %s", iter->first.c_str());
        for(unsigned int i = 0; i < iter->second.size(); i++) {
            g_Logger.debugLog(Logger::MSGID_EVENT, "\t\tG_SOURCE_ID : %u", iter->second[i]->m_timeoutId);
        }
    }
}
