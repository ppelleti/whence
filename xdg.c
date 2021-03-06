/*
 * Copyright (c) 2020 Patrick Pelletier
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "whence.h"

#ifndef _WIN32

/* For more information:
 * https://www.freedesktop.org/wiki/CommonExtendedAttributes/
 */

#include <stdlib.h>

static ErrorCode handle_attribute (const char *fname,
                                   const char *aname,
                                   char **dest,
                                   char **error) {
    char *result = NULL;
    size_t length = 0;

    const ErrorCode ec = getAttribute (fname, aname, &result, &length);
    if (ec == EC_OK && *dest == NULL) {
        *dest = result;
    } else if (ec > EC_NOATTR && *error == NULL) {
        *error = result;
    } else {
        free (result);
    }

    return ec;
}

#define ATTR(s, f) \
    handle_attribute (fname, (s), &dest->f, &dest->error)

#define A1(s, f) ErrorCode ec = ATTR(s, f)
#define AN(s, f) ec = combineErrors (ec, ATTR(s, f))

/* On MacOS, the getAttributes() in osx.c calls getAttributes_xdg(), so
 * that both MacOS and XDG attributes are supported.
 */
#ifdef __APPLE__
ErrorCode getAttributes_xdg (const char *fname,
                             Attributes *dest)
#else
ErrorCode getAttributes (const char *fname,
                         Attributes *dest,
                         Cache *cache)
#endif
{
    A1("user.xdg.origin.url", url);
    AN("user.xdg.referrer.url", referrer);
    AN("user.xdg.origin.email.from", from);
    AN("user.xdg.origin.email.subject", subject);
    AN("user.xdg.origin.email.message-id", message_id);
    AN("user.xdg.publisher", application);

    return ec;
}

#undef AN
#undef A1
#undef ATTR

#ifndef __APPLE__

/* On MacOS, these are defined in database.c instead.
 * For Linux and FreeBSD, they are no-ops. */

void Cache_init (Cache *cache) {
    // do nothing
}

void Cache_cleanup (Cache *cache) {
    // do nothing
}

#endif  /* not __APPLE__ */

#endif  /* not _WIN32 */
