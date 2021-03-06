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

#ifdef __APPLE__

/* For more information:
 * https://eclecticlight.co/2017/12/11/xattr-com-apple-quarantine-the-quarantine-flag/
 */

#include <sqlite3.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define DB_FILENAME \
    "/Library/Preferences/com.apple.LaunchServices.QuarantineEventsV2"

#define QUERY \
    "SELECT * FROM LSQuarantineEvent WHERE LSQuarantineEventIdentifier == '"

typedef struct DBCtx {
    Attributes *dest;
    const char *uuid;
} DBCtx;

static char *get_dbname (void) {
    uid_t u = getuid ();
    errno = 0;
    struct passwd *pw = getpwuid (u);
    if (pw == NULL) {
        return NULL;
    }

    ArrayList al;
    AL_init (&al);
    AL_add (&al, pw->pw_dir);
    AL_add (&al, DB_FILENAME);
    char *dbname = AL_join (&al);
    AL_cleanup (&al);

    endpwent ();

    return dbname;
}

static void handleKey (const char *key, const char *value, Attributes *dest) {
    char **field = NULL;

    if (key == NULL || value == NULL) {
        return;
    }

    if (0 == strcmp (key, "LSQuarantineOriginURLString")) {
        field = &dest->referrer;
    } else if (0 == strcmp (key, "LSQuarantineDataURLString")) {
        field = &dest->url;
    }

    if (field != NULL && *field == NULL) {
        *field = MY_STRDUP (value);
    }
}

static int callback (void *v, int nCols, char **values, char **names) {
    DBCtx *ctx = (DBCtx *) v;

    int i;
    for (i = 0; i < nCols; i++) {
        handleKey (names[i], values[i], ctx->dest);
    }

    return 0;
}

static ErrorCode run_query (Attributes *dest, const char *uuid, sqlite3 *sq) {
    char *errmsg = NULL;
    ErrorCode ec = EC_OK;
    DBCtx ctx;

    ctx.dest = dest;
    ctx.uuid = uuid;

    ArrayList al;
    AL_init (&al);
    AL_add (&al, QUERY);
    AL_add (&al, uuid);
    AL_add (&al, "'");
    char *query = AL_join (&al);
    AL_cleanup (&al);

    const int err = sqlite3_exec (sq, query, callback, &ctx, &errmsg);
    if (err != SQLITE_OK) {
        const char *msg = errmsg ? errmsg : "unknown";
        if (dest->error == NULL) {
            dest->error = MY_STRDUP (msg);
        }
        ec = EC_OTHER;
        goto done;
    }

 done:
    sqlite3_free (errmsg);
    return ec;
}

static sqlite3 *get_database (DatabaseConnection *conn) {
    if (conn->db) {
        return (sqlite3 *) conn->db;
    }

    if (conn->triedOpening) {
        return NULL;            /* so we don't try again if we already failed */
    }

    conn->triedOpening = true;

    char *dbname = get_dbname ();
    if (dbname == NULL) {
        perror (CMD_NAME);
        return NULL;
    }

    sqlite3 *sq = NULL;
    const int err = sqlite3_open_v2 (dbname, &sq, SQLITE_OPEN_READONLY, NULL);
    CHECK_NULL (sq);            /* should only be NULL if out of memory */
    if (err != SQLITE_OK) {
        err_printf ("%s: %s", dbname, sqlite3_errmsg (sq));
        sqlite3_close (sq);
        free (dbname);
        return NULL;
    }

    conn->db = sq;
    free (dbname);
    return sq;
}

ErrorCode lookup_uuid (Attributes *dest,
                       const char *uuid,
                       DatabaseConnection *conn) {
    sqlite3 *sq = get_database (conn);
    if (!sq) {
        return EC_OK;
    }

    return run_query (dest, uuid, sq);
}

void Cache_init (DatabaseConnection *conn) {
    memset (conn, 0, sizeof (*conn));
}

void Cache_cleanup (DatabaseConnection *conn) {
    if (conn->db) {
        sqlite3 *sq = (sqlite3 *) conn->db;
        sqlite3_close (sq);
        conn->db = NULL;
        conn->triedOpening = false;
    }
}

char *get_sqlite_version (void) {
    ArrayList al;

    AL_init (&al);
    AL_add (&al, "SQLite version ");
    AL_add (&al, sqlite3_libversion());
    AL_add (&al, " (header version " SQLITE_VERSION ")");
    char *ret = AL_join (&al);
    AL_cleanup (&al);

    return ret;
}

#endif  /* __APPLE__ */
