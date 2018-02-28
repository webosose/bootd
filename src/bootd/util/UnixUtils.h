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

#ifndef UNIXUTILS_H_
#define UNIXUTILS_H_

#include <fcntl.h>
#include <unistd.h>

inline bool isFileExist(const char *fileName)
{
    if (access(fileName, F_OK) == 0) {
        return true;
    }
    return false;
}

inline bool touchFile(const char *fileName)
{
    if (isFileExist(fileName)) {
        return true;
    }

    FILE *file = fopen(fileName, "w");
    if (file == NULL) {
        return false;
    }
    fclose(file);
    return true;
}

inline bool removeFile(const char *fileName)
{
    if (::unlink(fileName) == -1)
        return false;
    return true;
}
#endif /* UNIXUTILS_H_ */
