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
#include <sys/stat.h>
#include <unistd.h>

#include "Main.h"
#include "util/Logger.h"
#include "util/JUtil.h"

#include "StaticEventDB.h"

const char* StaticEventDB::DEBUG_CONF_FILE = WEBOS_INSTALL_LUNAPREFERENCESDIR "/bootd.json";

const char* StaticEventDB::KEY_LOGGER = "logger";
const char* StaticEventDB::KEY_LOGGER_LOG_TYPE = "logType";
const char* StaticEventDB::KEY_LOGGER_LOG_LEVEL = "logLevel";

StaticEventDB::StaticEventDB()
    : m_isNFSBoot(false),
      m_logLevel("debug")
{
    if (access(StaticEventDB::DEBUG_CONF_FILE, F_OK) == 0) {
        parseConfFile(StaticEventDB::DEBUG_CONF_FILE);
    } else {
        parseConfFile(PATH_DEFAULT_CONF);
    }
    parseCmdLine();
}

StaticEventDB::~StaticEventDB()
{

}

void StaticEventDB::printInformation()
{
    struct timespec basetime;

    g_Logger.getBaseTime(basetime);
    g_Logger.infoLog(Logger::MSGID_SETTINGS, "--DeviceType=%s", WEBOS_TARGET_MACHINE);
    g_Logger.infoLog(Logger::MSGID_SETTINGS, "--Distro=%s", WEBOS_TARGET_DISTRO);
    g_Logger.infoLog(Logger::MSGID_SETTINGS, "--DistroVariant=%s", WEBOS_TARGET_DISTRO_VARIANT);
    g_Logger.infoLog(Logger::MSGID_SETTINGS, "--NFSBoot=%s", isNFSBoot() ? "YES" : "NO");
    g_Logger.infoLog(Logger::MSGID_SETTINGS, "--TimeDiff=%ld.%ld(s) ", basetime.tv_sec, basetime.tv_nsec ? (basetime.tv_nsec / 1000000) : 0);
}

int StaticEventDB::getDisplayCnt()
{
    if (isFileExist("/sys/class/drm/card0-HDMI-A-2"))
        return 2;
    else
        return 1;
}

void StaticEventDB::updateConf(pbnjson::JValue jsonConf)
{
    if (jsonConf.hasKey(KEY_LOGGER)) {
        if (jsonConf[KEY_LOGGER].hasKey(KEY_LOGGER_LOG_TYPE)) {
            for (JValue logtype : jsonConf[KEY_LOGGER][KEY_LOGGER_LOG_TYPE].items()) {
                g_Logger.enableLogType(g_Logger.getLogType(logtype.asString()));
            }
        }
        if (jsonConf[KEY_LOGGER].hasKey(KEY_LOGGER_LOG_LEVEL)) {
            jsonConf[KEY_LOGGER][KEY_LOGGER_LOG_LEVEL].asString(m_logLevel);
            g_Logger.changeLogLevel(g_Logger.getLogLevel(m_logLevel));
        }
    }
}

void StaticEventDB::parseConfFile(string file)
{
    g_Logger.infoLog(Logger::MSGID_SETTINGS, "Read configuration (%s)", file.c_str());
    JValue jsonConf = JUtil::parseFile(file.c_str());

    // 1. Common Configuration
    if (jsonConf.hasKey("common")) {
        updateConf(jsonConf["common"]);
    }
    // 2. Machine Configuration
    if (jsonConf.hasKey(WEBOS_TARGET_MACHINE)) {
        updateConf(jsonConf[WEBOS_TARGET_MACHINE]);
    }
    // 3. Distro Configuration
    if (jsonConf.hasKey(WEBOS_TARGET_DISTRO)) {
        updateConf(jsonConf[WEBOS_TARGET_DISTRO]);
    }
    // 4. Distro-variant Configuration
    if (jsonConf.hasKey(WEBOS_TARGET_DISTRO_VARIANT)) {
        updateConf(jsonConf[WEBOS_TARGET_DISTRO_VARIANT]);
    }
}

void StaticEventDB::parseCmdLine()
{
    ifstream file;
    string cmdline, hostname;

    file.open("/proc/cmdline");
    if (file.fail()) {
        g_Logger.errorLog(Logger::MSGID_SETTINGS, "%s=File Open Failed(/proc/cmdline)", __FUNCTION__);
    } else {
        cmdline.assign((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

        if (std::string::npos != cmdline.find("nfsroot")) {
            m_isNFSBoot = true;
        }
        file.close();
    }

    file.open("/etc/hostname");
    if (file.fail()) {
        g_Logger.errorLog(Logger::MSGID_SETTINGS, "%s=File Open Failed(/etc/hostname)", __FUNCTION__);
    } else {
        hostname.assign((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        file.close();
    }
}
