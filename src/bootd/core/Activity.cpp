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

#include "util/Logger.h"

#include "Activity.h"

Activity::Activity()
    : m_id(0)
{
}

Activity::Activity(JValue activity)
    : Activity()
{
    m_object = activity.duplicate();
    m_object["name"].asString(m_name);
}

Activity::~Activity()
{
}

void Activity::setId(int id)
{
    m_id = id;
}

int Activity::getId()
{
    return m_id;
}

string& Activity::getName()
{
    return m_name;
}

JValue& Activity::getObject()
{
    return m_object;
}
