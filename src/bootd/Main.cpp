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

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <iostream>
#include <memory>

#include <glib.h>

#include "sequencer/BootSequencerFactory.h"
#include "util/Logger.h"

#include "Main.h"

static AbsBootSequencer *s_sequencer = NULL;

void sig_handler(int signal)
{
    if (s_sequencer != NULL) {
        s_sequencer->stop();
    }
}

int main(int argc, char **argv)
{
    g_Logger.performanceLog(Logger::MSGID_BOOT, "BOOTD_START");

    // handle signal for shutdown command
    signal(SIGTERM, sig_handler);

    try {
        s_sequencer = BootSequencerFactory::getBootSequencer();
        s_sequencer->start();
    } catch (const std::exception& e) {
        g_Logger.errorLog(Logger::MSGID_GENERAL, "Initialization failed=Message(%s)", e.what());
    }
    g_Logger.performanceLog(Logger::MSGID_BOOT, "BOOTD_END");
    return EXIT_SUCCESS;
}
