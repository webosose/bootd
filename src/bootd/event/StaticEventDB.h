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

#ifndef STATICEVENTDB_H_
#define STATICEVENTDB_H_

#include <iostream>
#include <map>
#include <queue>
#include <set>
#include <vector>

#include <luna-service2/lunaservice.h>
#include <pbnjson.hpp>

#include "Main.h"
#include "util/JUtil.h"
#include "util/UnixUtils.h"

using namespace std;
using namespace pbnjson;

enum BootStatus {
    BootStatus_unknown,
    BootStatus_normal,
};

enum PowerStatus {
    PowerStatus_unknown,
    PowerStatus_active,
    PowerStatus_activeStandby,
    PowerStatus_suspend,
};

enum BootTarget {
    BootTarget_unknown,
    BootTarget_hardware,
    BootTarget_emulator,
};

class StaticEventDB {
public:
    static StaticEventDB* instance()
    {
        static StaticEventDB _instance;
        return &_instance;
    }

    virtual ~StaticEventDB();

    void printInformation();

    bool isNFSBoot()
    {
        return m_isNFSBoot;
    }

    int getDisplayCnt();

private:
    StaticEventDB();

    void updateConf(pbnjson::JValue jsonConf);

    void parseConfFile(string file);
    void parseCmdLine();

    static const char *DEBUG_CONF_FILE;

    static const char *KEY_LOGGER;
    static const char *KEY_LOGGER_LOG_TYPE;
    static const char *KEY_LOGGER_LOG_LEVEL;

    bool m_isNFSBoot;

    string m_logType;
    string m_logLevel;
};

#endif /* STATICEVENTDB_H_ */
