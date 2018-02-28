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

#ifndef COMMAND_H_
#define COMMAND_H_

#include <deque>
#include <iostream>

#include <glib.h>

#include "Main.h"

using namespace std;

class Command {
public:
    Command(string command);
    virtual ~Command();

    void addArg(string arg);
    bool exec();
    void wait();
    void wait(GMainLoop* mainLoop);
    int getExitCode();

private:
    string m_comm;
    deque<string> m_args;

    pid_t m_pid;
    int m_status;
    bool m_isInLoop;

    static void _wait(GPid pid, gint status, void* ctx);
};

#endif /* COMMAND_H_ */
