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

#include "apr.h"
#include "apr_strings.h"
#include "apr_portable.h"


APR_DECLARE(const char*) apr_os_default_encoding (apr_pool_t *pool)
{
    return apr_psprintf(pool, "CP%u", (unsigned) GetACP());
}


APR_DECLARE(const char*) apr_os_locale_encoding (apr_pool_t *pool)
{
    LCID locale = GetThreadLocale();
    int len = GetLocaleInfo(locale, LOCALE_IDEFAULTANSICODEPAGE, NULL, 0);
    char *cp = apr_palloc(pool, len + 2);
    if (0 < GetLocaleInfo(locale, LOCALE_IDEFAULTANSICODEPAGE, cp + 2, len))
    {
        /* Fix up the returned number to make a valid codepage name of
          the form "CPnnnn". */
        cp[0] = 'C';
        cp[1] = 'P';
        return cp;
    }

    return apr_os_default_encoding(pool);
}
