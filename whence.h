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

#ifndef WHENCE_H
#define WHENCE_H

#include <stddef.h>
#include <time.h>
#include <stdbool.h>
#include <stdint.h>

#define CMD_NAME "whence"
#define CMD_VERSION "0.9"

typedef enum ErrorCode {
    EC_OK = 0,
    EC_NOATTR = 1,
    EC_NOFILE = 2,
    EC_OTHER = 3,
    EC_CMDLINE = 4,
    EC_MEM = 5
} ErrorCode;

typedef struct ArrayList {
    char **strings;
    size_t size;
    size_t capacity;
} ArrayList;

typedef struct Attributes {
    char *url;
    char *referrer;
    char *from;
    char *subject;
    char *message_id;
    char *application;
    time_t date;
    char *zone;
    char *error;
} Attributes;

typedef enum AttrStyle {
    AS_HUMAN,
    AS_HUMAN_COLOR,
    AS_JSON_NOTLAST,
    AS_JSON_LAST
} AttrStyle;

typedef struct DatabaseConnection {
    void *db;
    bool triedOpening;
} DatabaseConnection;

typedef struct ZoneCache {
    ArrayList keys;
    ArrayList values;
} ZoneCache;

#ifdef __APPLE__
typedef DatabaseConnection Cache;
#elif defined (_WIN32)
typedef ZoneCache Cache;
#else
typedef int Cache;              /* dummy */
#endif

#define CHECK_NULL(x)                                           \
    do { if ((x) == NULL) oom (__FILE__, __LINE__); } while (0)

#define MY_STRDUP(x) my_strdup ((x), __FILE__, __LINE__)

/* getattr.c or windows.c */
ErrorCode getAttribute (const char *fname,
                        const char *attr,
                        char **result,
                        size_t *length);
char *fixFilename (const char *fname, int32_t *drives);

/* util.c */
extern bool colorize_errors;
void oom (const char *file, long line);
ErrorCode combineErrors (ErrorCode ec1, ErrorCode ec2);
char *my_strdup (const char *s, const char *file, long line);
void err_printf (const char *fmt, ...)
#ifdef __GNUC__
    __attribute__ ((format (printf, 1, 2)))
#endif
    ;

/* array-list.c */
void AL_init (ArrayList *al);
void AL_add (ArrayList *al, const char *str);
void AL_add_nocopy (ArrayList *al, char *str);
char *AL_join (const ArrayList *al);
void AL_clear (ArrayList *al);
void AL_cleanup (ArrayList *al);

/* props.c */
ErrorCode props2list (const void *data, size_t length, ArrayList *dest);

/* split.c */
void split (const char *str, char sep, ArrayList *dest);

/* attributes.c */
void Attr_init (Attributes *attrs);
void Attr_print (const Attributes *attrs, const char *fname, AttrStyle style);
void Attr_cleanup (Attributes *attrs);

/* linux.c, macos.c, or windows.c */
ErrorCode getAttributes (const char *fname,
                         Attributes *dest,
                         Cache *cache);

/* linux.c, database.c, or registry.c */
void Cache_init (Cache *cache);
void Cache_cleanup (Cache *cache);

/* color.c */
bool enableColorEscapes (int fd);

/* database.c */
ErrorCode lookup_uuid (Attributes *dest,
                       const char *uuid,
                       DatabaseConnection *conn);
char *get_sqlite_version (void);

/* registry.c */
const char *getZoneName (const char *zoneNumber, ZoneCache *zc);

#endif  /* WHENCE_H */
