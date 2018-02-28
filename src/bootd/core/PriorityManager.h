// Copyright (c) 2014-2018 LG Electronics, Inc.
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

#ifndef PRIORITYMANAGER_H_
#define PRIORITYMANAGER_H_

#include <iostream>
#include <map>

#include <glib.h>

#include "util/JUtil.h"

using namespace std;
using namespace pbnjson;

struct ProcessInfo {
    int pid;
    int ppid;
    int pgrp;
};

class PriorityManager {
public:
    static PriorityManager* instance()
    {
        static PriorityManager _instance;
        return &_instance;
    }

    virtual ~PriorityManager();

    bool adjustNiceValue(JValue priorities);

private:
    PriorityManager();

    bool getProcessInfo(string procStatPath, string &comm, struct ProcessInfo &processInfo);

    bool refreshProcessInfoMap();
    void printProcessInfoMap();

    bool adjustProcessPriority(string processName, int nice);

    static const int RETRY_COUNT = 2;

    map<string, struct ProcessInfo> m_processInfoMap;

};

#endif /* PRIORITYMANAGER_H_ */
