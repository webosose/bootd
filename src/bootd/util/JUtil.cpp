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

#include <fstream>

#include "JUtil.h"
#include "Logger.h"

class DefaultResolver: public pbnjson::JResolver {
public:
    pbnjson::JSchema resolve(const ResolutionRequest &request, JSchemaResolutionResult &result)
    {
        pbnjson::JSchema resolved = pbnjson::JSchema::AllSchema();
        if (!resolved.isInitialized()) {
            g_Logger.errorLog(Logger::MSGID_GENERAL, "json resolve=%s", request.resource().c_str());
            result = SCHEMA_IO_ERROR;
            return pbnjson::JSchema::NullSchema();
        }

        result = SCHEMA_RESOLVED;
        return resolved;
    }
};

pbnjson::JValue JUtil::parse(string str)
{
    return JDomParser::fromString(str);
}

pbnjson::JValue JUtil::parseFile(const char *file)
{
    return JDomParser::fromFile(file);
}

void JUtil::getValueWithKeys(JValue root, JValue keys, JValue &value)
{
    JValue current = std::move(root);
    std::string subKey;

    for (int i = 0; i < keys.arraySize(); i++) {
        subKey = keys[i].asString();

        if (current.isObject()) {
            current = current[subKey];
        } else {
            for (int j = 0; j < current.arraySize(); j++) {
                current = current[j][subKey];
            }
        }
    }

    value = std::move(current);
}

bool JUtil::isNotNull(JValue &value)
{
    return (value.isArray() && value.arraySize() == 0) ? false : true;
}

bool JUtil::isEqual(JValue operand1, JValue operand2)
{
    return (operand1 == operand2);
}

bool JUtil::isNotEqual(JValue operand1, JValue operand2)
{
    return (operand1 != operand2);
}

JUtil::Error::Error()
    : m_code(Error::None)
{
}

JUtil::Error::ErrorCode JUtil::Error::code()
{
    return m_code;
}

std::string JUtil::Error::detail()
{
    return m_detail;
}

void JUtil::Error::set(ErrorCode code, const char *detail)
{
    m_code = code;
    if (!detail) {
        switch (m_code) {
        case Error::None:
            m_detail = "Success";
            break;
        case Error::File_Io:
            m_detail = "Fail to read file";
            break;
        case Error::Schema:
            m_detail = "Fail to read schema";
            break;
        case Error::Parse:
            m_detail = "Fail to parse json";
            break;
        default:
            m_detail = "Unknown error";
            break;
        }
    } else {
        m_detail = detail;
    }
}
