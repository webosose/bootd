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

#ifndef ACTIVITYMANAGER_H_
#define ACTIVITYMANAGER_H_

#include <iostream>

#include <luna-service2++/call.hpp>
#include <luna-service2++/handle.hpp>

#include "AbsService.h"

using namespace std;
using namespace LS;

class Activity;

class ActivityManager : public AbsService {
public:
    static ActivityManager* instance()
    {
        static ActivityManager _instance;
        return &_instance;
    }

    virtual ~ActivityManager();

    bool restart(Handle *handle, Activity &activity);
    bool create(Handle *handle, Activity &activity);

private:
    static const string SERVICE_NAME;

    ActivityManager();

};

#endif /* ACTIVITYMANAGER_H_ */
