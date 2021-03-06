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
#include <stdlib.h>

#define SUBKEY \
    "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Internet Settings\\Zones\\"

static char *get_reg (HKEY key, const char *subkey) {
    const char *value = "DisplayName";
    DWORD len = 0;
    utf16 *wresult = NULL;
    char *result = NULL;

    utf16 *wsubkey = utf8to16_nofail (subkey);
    utf16 *wvalue = utf8to16_nofail (value);

    LSTATUS st = RegGetValue (key, wsubkey, wvalue,
                              RRF_RT_REG_SZ, NULL, NULL, &len);
    if (st != ERROR_SUCCESS || len == 0) {
        goto done;
    }

    wresult = malloc (len);
    CHECK_NULL (wresult);
    st = RegGetValue (key, wsubkey, wvalue,
                      RRF_RT_REG_SZ, NULL, wresult, &len);
    if (st != ERROR_SUCCESS) {
        goto done;
    }

    result = utf16to8_nofail (wresult);

 done:
    free (wresult);
    free (wvalue);
    free (wsubkey);

    return result;
}

static char *lookupZoneName (const char *zoneNumber) {
    ArrayList al;

    AL_init (&al);
    AL_add (&al, SUBKEY);
    AL_add (&al, zoneNumber);
    char *subkey = AL_join (&al);
    AL_cleanup (&al);

    char *name = get_reg (HKEY_CURRENT_USER, subkey);
    if (name == NULL) {
        name = get_reg (HKEY_LOCAL_MACHINE, subkey);
    }

    free (subkey);
    return name;
}

const char *getZoneName (const char *zoneNumber, ZoneCache *zc) {
    size_t i;

    /* linear search for cached zone */
    for (i = 0; i < zc->keys.size; i++) {
        if (0 == strcmp (zoneNumber, zc->keys.strings[i])) {
            return (zc->values.strings[i]);
        }
    }

    if (zc->keys.size >= 100) {
        /* This is incredibly unlikely to happen, since typically there are
         * about five zones or so.  But if we were for some reason caching
         * a large number of zones, clear it out so we don't get O(n^2)
         * behavior with linear search. */
        AL_clear (&zc->keys);
        AL_clear (&zc->values);
    }

    char *name = lookupZoneName (zoneNumber);
    AL_add (&zc->keys, zoneNumber);
    if (name) {
        AL_add_nocopy (&zc->values, name);
    } else {
        /* If no name found, cache the zone number as the name,
         * so we don't keep looking it up every time. */
        AL_add (&zc->values, zoneNumber);
    }

    return (name ? name : zoneNumber);
}

void Cache_init (ZoneCache *cache) {
    AL_init (&cache->keys);
    AL_init (&cache->values);
}

void Cache_cleanup (ZoneCache *cache) {
    AL_cleanup (&cache->keys);
    AL_cleanup (&cache->values);
}

#endif  /* _WIN32 */
