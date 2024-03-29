/* Copyright 2001-2004 The Apache Software Foundation
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

#ifndef APACHE_MPM_MPMT_OS2_H
#define APACHE_MPM_MPMT_OS2_H

#define MPMT_OS2_MPM

#include "httpd.h"
#include "mpm_default.h"
#include "scoreboard.h"

#define MPM_NAME "MPMT_OS2"

extern server_rec *ap_server_conf;
#define AP_MPM_WANT_SET_PIDFILE
#define AP_MPM_WANT_SET_MAX_REQUESTS
#define AP_MPM_DISABLE_NAGLE_ACCEPTED_SOCK
#define AP_MPM_WANT_SET_MAX_MEM_FREE

#endif /* APACHE_MPM_MPMT_OS2_H */
