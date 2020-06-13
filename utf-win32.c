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

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <string.h>
#include <wchar.h>

utf16 *utf8to16 (const char *s) {
    return utf8to16_len (s, strlen (s));
}

char *utf16to8 (const utf16 *s) {
    return utf16to8_len (s, wcslen (s));
}

utf16 *utf8to16_len (const char *s, size_t len) {
    if (len == 0) {             /* special case for empty string */
        utf16 *result = malloc (2);
        CHECK_NULL (result);
        *result = 0;
        return result;
    }

    const size_t result_code_units = len + 1;
    utf16 *result = (utf16 *) malloc (2 * result_code_units);
    CHECK_NULL (result);
    const int ret =
        MultiByteToWideChar (CP_UTF8, 0, s, len, result, result_code_units);

    if (ret <= 0 || ret >= result_code_units - 1) {
        free (result);
        return NULL;
    }

    result[ret] = 0;            /* NUL terminate result */
    const size_t actual_code_units = ret + 1;
    result = (utf16 *) realloc (2 * actual_code_units);
    CHECK_NULL (result);
    return result;
}

char *utf16to8_len (const utf16 *s, size_t len) {
    if (len == 0) {             /* special case for empty string */
        char *result = malloc (1);
        CHECK_NULL (result);
        *result = 0;
        return result;
    }

    const size_t result_bytes = len * 3 + 1;
    char *result = malloc (result_bytes);
    CHECK_NULL (result);
    const int ret = WideCharToMultiByte (CP_UTF8, 0, s, len,
                                         result, result_bytes, NULL, NULL);

    if (ret <= 0 || ret >= result_bytes - 1) {
        free (result);
        return NULL;
    }

    result[ret] = 0;            /* NUL terminate result */
    const size_t actual_bytes = ret + 1;
    result = realloc (actual_bytes);
    CHECK_NULL (result);
    return result;
}

#endif  /* _WIN32 */
