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

#ifndef SURFACEMANAGER_H_
#define SURFACEMANAGER_H_

#include <iostream>

#include <luna-service2++/call.hpp>
#include <luna-service2++/handle.hpp>
#include <pbnjson.hpp>

#include "AbsService.h"

using namespace std;
using namespace LS;
using namespace pbnjson;

class SurfaceManager;

class SurfaceManagerListener {
public:
    friend class SurfaceManager;

    SurfaceManagerListener() {}
    SurfaceManagerListener(const SurfaceManagerListener &a) {}
    virtual ~SurfaceManagerListener() {};

    virtual void onSurfaceManagerCategoryChange(JValue &response) {};

protected:
    Call m_call;
};

class SurfaceManager : public AbsService {
public:
    static SurfaceManager* instance()
    {
        static SurfaceManager _instance;
        return &_instance;
    }

    virtual ~SurfaceManager();

    bool registerServiceCategory(Handle *handle, SurfaceManagerListener *listener);
    bool showSpinner(Handle *handle, bool show);

private:
    static bool _registerServiceCategory(LSHandle *sh, LSMessage *reply, void *ctx);

    SurfaceManager();

    static const string SERVICE_NAME;
};

#endif /* SURFACEMANAGER_H_ */
