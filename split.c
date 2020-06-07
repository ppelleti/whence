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

#include <string.h>
#include <stdlib.h>

void split (const char *str, char sep, ArrayList *dest) {
    const char *s = str;

    for ( ; ; ) {
        const char *p = strchr (s, sep);
        if (p == NULL) {
            AL_add (dest, s);
            break;
        } else {
            const size_t len = p - s;
            char *newStr = malloc (len + 1);
            CHECK_NULL (newStr);
            memcpy (newStr, s, len);
            newStr[len] = 0;
            AL_add_nocopy (dest, newStr);
            s = p + 1;
        }
    }
}
