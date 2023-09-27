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

#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "util/Logger.h"

#include "Command.h"

Command::Command(string command)
    : m_comm(command),
      m_args(),
      m_pid(-1),
      m_status(-1),
      m_isInLoop(false)
{
}

Command::~Command()
{
}

void Command::addArg(string arg)
{
    m_args.push_back(arg);
}

bool Command::exec()
{
    std::size_t memsize = 0;

    std::size_t size = m_args.size() + 2; //CID 9006407 CERT-C Integers

    if (size < m_args.size())
        return false;
    else
        memsize = sizeof(char*) * size;

    char **command = (char **)malloc(memsize);
    if (command)
        memset(command, 0, memsize);    //coverity CID 9154143
    else
        return false;

    command[0] = &*m_comm.begin();//m_comm.c_str());
    for (unsigned int i = 0; i < m_args.size(); i++) {
        command[i+1] = &*(m_args.at(i).begin());    //coverity CID-9042669
    }

    g_Logger.debugLog(Logger::MSGID_UTIL, "Forking %s", m_comm.c_str());

    pid_t pid = fork();
    if (pid == -1) {
        free(command);
        return false;
    } else if (pid > 0) {
        free(command);
        m_pid = pid;
        return true;
    }

    // child process
    execv(m_comm.c_str(), command);
    free(command); // for static analysis

    // something is wrong
    g_Logger.errorLog(Logger::MSGID_UTIL, 0, "Creation of child process fails");
    return false;
}

void Command::wait()
{
    g_Logger.debugLog(Logger::MSGID_UTIL, "Waiting %s's exit", m_comm.c_str());

    if (m_pid <= 0) {
        g_Logger.errorLog(Logger::MSGID_UTIL, 0, "Command doesn't start yet");
        return;
    }
    if (waitpid(m_pid, &m_status, WUNTRACED | WCONTINUED) <= 0) {
        g_Logger.errorLog(Logger::MSGID_UTIL, 0, "Fails to wait child process");
        return;
    }
}

void Command::_wait(GPid pid, gint status, void* ctx)
{
    g_Logger.debugLog(Logger::MSGID_UTIL, "Resourcemon work done");
    Command* comm = (Command*)ctx;
    g_spawn_close_pid(pid);

    if (!comm->m_isInLoop) {
        g_Logger.errorLog(Logger::MSGID_UTIL,  "Child process is done without loop. This might be a bug");
    } else {
        comm->m_isInLoop = false;
    }
    comm->m_status = status;
}

void Command::wait(GMainLoop* mainLoop)
{
    GSource *source = g_child_watch_source_new(m_pid);
    GMainContext* ctx = g_main_loop_get_context(mainLoop);

    if (ctx == NULL || source == NULL) {
        g_Logger.errorLog(Logger::MSGID_UTIL, "Get source / context fail");
        return;
    }
    g_source_set_callback(source, (GSourceFunc)_wait, this, NULL);
    g_source_attach(source, ctx);

    m_isInLoop = true;
    while (m_isInLoop) {
            g_main_context_iteration(g_main_loop_get_context(mainLoop), TRUE);
    }

    g_source_destroy(source);
    g_source_unref(source);

    return;
}

int Command::getExitCode()
{
    if (m_status < 0) {
        g_Logger.errorLog(Logger::MSGID_UTIL, 0, "Unknown status. Did you call wait function?");
        return -1;
    }
    return WEXITSTATUS(m_status);
}
