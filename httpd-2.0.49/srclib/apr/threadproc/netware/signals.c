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

#include "apr_arch_threadproc.h"
#include <nks/thread.h>
#include "apr_private.h"
#include "apr_pools.h"
#include "apr_signal.h"
#include "apr_strings.h"

#include <assert.h>
#if APR_HAS_THREADS && APR_HAVE_PTHREAD_H
#include <pthread.h>
#endif

APR_DECLARE(apr_status_t) apr_proc_kill(apr_proc_t *proc, int signum)
{
    return APR_ENOTIMPL;
}


void apr_signal_init(apr_pool_t *pglobal)
{
}

const char *apr_signal_description_get(int signum)
{
    switch (signum)
    {
    case SIGABRT:
        return "Abort";
    case SIGFPE:
        return "Arithmetic exception";
    case SIGILL:
        return "Illegal instruction";
    case SIGINT:
        return "Interrupt";
    case SIGSEGV:
        return "Segmentation fault";
    case SIGTERM:
        return "Terminated";
    case SIGPOLL:
        return "Pollable event occurred";
    default:
        return "unknown signal (not supported)";
    }
}

static void *signal_thread_func(void *signal_handler)
{
    return NULL;
}

APR_DECLARE(apr_status_t) apr_setup_signal_thread(void)
{
    int rv = 0;

    return rv;
}

/* Deprecated */
const char *apr_signal_get_description(int signum)
{
    return apr_signal_description_get(signum);
}
