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

#ifndef PROC_MUTEX_H
#define PROC_MUTEX_H

#include "apr_pools.h"
#include "apr_proc_mutex.h"
#include "apr_file_io.h"
#include "apr_general.h"
#include "apr_lib.h"
#include "apr_portable.h"

struct apr_proc_mutex_t {
    apr_pool_t *pool;
    
    /* Our lock :) */
    sem_id Lock;
    int32  LockCount;
};

#endif  /* PROC_MUTEX_H */

