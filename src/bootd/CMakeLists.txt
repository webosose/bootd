# Copyright (c) 2015-2021 LG Electronics, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# SPDX-License-Identifier: Apache-2.0

include(FindPkgConfig)

pkg_check_modules(GLIB2 REQUIRED glib-2.0)
include_directories(${GLIB2_INCLUDE_DIRS})
webos_add_compiler_flags(ALL ${GLIB2_CFLAGS_OTHER})

pkg_check_modules(GIO2 REQUIRED gio-2.0)
include_directories(${GIO2_INCLUDE_DIRS})
webos_add_compiler_flags(ALL ${GIO2_CFLAGS_OTHER})

pkg_check_modules(LUNASERVICE2 REQUIRED luna-service2)
include_directories(${LUNASERVICE2_INCLUDE_DIRS})
webos_add_compiler_flags(ALL ${LUNASERVICE2_CFLAGS_OTHER})

pkg_check_modules(PBNJSON_CPP REQUIRED pbnjson_cpp)
include_directories(${PBNJSON_CPP_INCLUDE_DIRS})
webos_add_compiler_flags(ALL ${PBNJSON_CPP_CFLAGS_OTHER})

pkg_check_modules(PMLOG REQUIRED PmLogLib)
include_directories(${PMLOG_INCLUDE_DIRS})
webos_add_compiler_flags(ALL ${PMLOG_CFLAGS_OTHER})

find_package(Boost REQUIRED COMPONENTS regex system)
include_directories(${Boost_INCLUDE_DIRS})
webos_add_compiler_flags(ALL ${Boost_CFLAGS_OTHER})

pkg_check_modules(LUNASERVICE2PP REQUIRED luna-service2++)
include_directories(${LUNASERVICE2PP_INCLUDE_DIRS})
webos_add_compiler_flags(ALL ${LUNASERVICE2PP_CFLAGS_OTHER})

# Basic
set(BIN_NAME bootd)
file(GLOB_RECURSE SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

# Compiling
webos_add_compiler_flags(ALL -Wall "-std=gnu++0x" -fthreadsafe-statics)
webos_configure_source_files(cfg ${CMAKE_CURRENT_SOURCE_DIR}/Environment.h)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${PROJECT_BINARY_DIR}/Configured/src/bootd)
add_executable(${BIN_NAME} ${SRC_FILES} ${LTTNG_SOURCE})

# Linking
webos_add_linker_options(ALL --no-undefined)
set(LIBS ${GLIB2_LDFLAGS}
    ${GIO2_LDFLAGS}
    ${PBNJSON_CPP_LDFLAGS}
    ${PMLOG_LDFLAGS}
    ${Boost_LIBRARIES}
    ${RT}
    ${LTTNG_LDFLAGS}
    ${LUNASERVICE2_LDFLAGS}
    ${LUNASERVICE2PP_LDFLAGS}
)
target_link_libraries(${BIN_NAME} ${LIBS})

# Install
webos_build_program(NAME ${BIN_NAME})
install(TARGETS ${BIN_NAME} DESTINATION ${WEBOS_INSTALL_ROOT}/usr/sbin)
webos_build_configured_file(files/conf/bootd.json WEBOS_SYSCONFDIR "")
