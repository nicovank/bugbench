/* Copyright 2000-2004 The Apache Software Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef THREAD_MUTEX_H
#define THREAD_MUTEX_H

#include "apr_thread_mutex.h"
#include "apr_file_io.h"

struct apr_thread_mutex_t {
    apr_pool_t *pool;
    HMTX hMutex;
};

#endif  /* THREAD_MUTEX_H */

