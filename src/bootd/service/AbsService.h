// Copyright (c) 2015-2018 LG Electronics, Inc.
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


#ifndef ABSSERVICE_H_
#define ABSSERVICE_H_

#include <iostream>

#include <luna-service2++/call.hpp>
#include <luna-service2++/handle.hpp>

#include "util/Logger.h"

using namespace std;
using namespace LS;

class AbsService {
public:
    static bool getServerStatus(Handle *handle, string serviceName);

    AbsService(string name);
    virtual ~AbsService();

    string& getName();
    int getTimeout();

    bool registerServerStatus(Handle *handle, ServerStatusCallback callback);

protected:
    static const int TIMEOUT_MAX = 5000;

private:
    string m_name;
    ServerStatusCallback m_callback;
    ServerStatus m_serverStatus;
};

#endif /* ABSSERVICE_H_ */
