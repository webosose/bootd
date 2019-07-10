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

#ifndef DYNAMICEVENTDB_H_
#define DYNAMICEVENTDB_H_

#include <iostream>
#include <map>
#include <vector>

#include <gio/gio.h>
#include <glib.h>

#include "Main.h"
#include "util/JUtil.h"

using namespace std;

enum EventCoreTimeout {
    EventCoreTimeout_Zero = 0,
    EventCoreTimeout_One = 1,
    EventCoreTimeout_Three = 3,
    EventCoreTimeout_Min = 5,
    EventCoreTimeout_Middle = 15,
    EventCoreTimeout_Max = 30,
    EventCoreTimeout_One_Minute = 60,
    EventCoreTimeout_Three_Minutes = 180,
    EventCoreTimeout_Endless = 1000,
};

enum EventHandleType {
    TriggerEvent = 0,
    ClearEvent = 1,
};

class DynamicEventDBListener {
public:
    DynamicEventDBListener() {};
    virtual ~DynamicEventDBListener() {};

    // interfaces
    virtual void onTriggerEvent(string eventName = "") {};
    virtual void onClearEvent(string eventName = "") {};
    virtual void onWaitAsyncEvent(string eventName = "") {};
};

struct AsyncEventData {
    string m_eventName;
    DynamicEventDBListener* m_listener;
    guint m_timeoutId;
};

class DynamicEventDB {
public:
    // Reserved EVENTS
    static const char *EVENT_BOOT_COMPLETE;
    static const char *EVENT_FIRSTAPP_LAUNCHED;

    static DynamicEventDB* instance()
    {
        static DynamicEventDB _instance;
        return &_instance;
    }

    virtual ~DynamicEventDB();

    bool isWaitSomething();
    bool hasEvent(string eventName);
    bool getEventStatus(string eventName);
    bool getEventStatus(string eventName, JValue &extra);
    bool replaceEventsInfo(JValue &events, string origin, string &result);
    bool removeEvent(string eventName);

    bool waitTimeout(GMainLoop* mainLoop, int seconds);

    bool existEvent(GMainLoop* mainLoop, string eventName, int seconds);
    bool waitEvent(GMainLoop* mainLoop, string eventName, int seconds);
    bool waitAsyncEvent(GMainLoop* mainLoop, string eventName, int seconds, DynamicEventDBListener* listener);
    bool triggerEvent(string eventName, JValue extra = pbnjson::Object());
    bool clearEvent(string eventName);

    bool waitFile(GMainLoop* mainLoop, string filePath, int seconds);

private:
    static int _waitTimeout(void* ctx);
    static int _waitEventTimeout(void* ctx);
    static int _waitAsyncEventTimeout(void* ctx);
    static int _waitFileTimeout(void* ctx);
    static void _waitFile(GFileMonitor *monitor, GFile *file, GFile *other_file, GFileMonitorEvent event_type, gpointer user_data);

    DynamicEventDB();

    void callEventListeners(string eventName, EventHandleType type);
    void callAsyncEventListeners(string eventName);
    void cleanFileMonitor();
    void printAsyncEventMap();

    static const char *KEY_READY;
    static const char *KEY_TIME;
    static const char *KEY_EXTRA;

    bool m_isInLoop;

    guint m_waitTimeoutId;
    guint m_waitEventTimeoutId;
    guint m_waitFileTimeoutId;

    pbnjson::JValue m_eventDatabase;
    map<string, vector<AsyncEventData*>> m_asyncEventData;

    // TODO: Need to removed in the future
    GFile *m_dir;
    GFileMonitor *m_dirMonitor;
    int m_dirMonitorId;
    string m_waitEvent;
    string m_existEvent;
};

#endif /* DYNAMICEVENTDB_H_ */
