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

#ifndef GENERICSERVICE_H_
#define GENERICSERVICE_H_

#include <iostream>

#include <luna-service2++/call.hpp>
#include <luna-service2++/handle.hpp>
#include <pbnjson.hpp>

#include "AbsService.h"

using namespace std;
using namespace LS;
using namespace pbnjson;

class GenericService;
class AbsAction;
class AbsCondition;

class GenericServiceListener {
public:
    friend class GenericService;

    GenericServiceListener() {};
    GenericServiceListener(const GenericServiceListener &a) {}
    virtual ~GenericServiceListener() {};

    virtual void onSubscription(JValue &response) {};

protected:
    Call m_call;
};

class GenericService : public AbsService {
public:
    static GenericService* instance()
    {
        static GenericService _instance;
        return &_instance;
    }

    virtual ~GenericService();

private:
    static bool _callMultiReply(LSHandle *sh, LSMessage *reply, void *ctx);

    GenericService();

    static const string SERVICE_NAME;
};

#endif /* GENERICSERVICE_H_ */
