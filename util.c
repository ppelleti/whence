#include "whence.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

bool colorize_errors = false;

static const char *my_basename (const char *file) {
    const char *slash = strrchr (file, '/');
    if (slash) {
        return slash + 1;
    } else {
        return file;
    }
}

void oom (const char *file, long line) {
    err_printf (CMD_NAME ": out of memory at %s:%ld",
                my_basename (file), line);
    exit (EC_MEM);
}

ErrorCode combineErrors (ErrorCode ec1, ErrorCode ec2) {
    if (ec2 > ec1 && ec2 > EC_NOATTR) {
        return ec2;
    } else if (ec1 > EC_NOATTR) {
        return ec1;
    } else if (ec2 == EC_OK) {
        return EC_OK;
    } else {
        return ec1;
    }
}

char *my_strdup (const char *s, const char *file, long line) {
    if (s == NULL) {
        err_printf (CMD_NAME ": strdup called on NULL at %s:%ld",
                    my_basename (file), line);
        exit (EC_OTHER);
    }

    char *ret = strdup (s);
    if (ret == NULL) {
        oom (file, line);
    }

    return ret;
}

void err_printf (const char *format, ...) {
    va_list va;
    if (colorize_errors) {
        fprintf (stderr, "\e[91m");
    }
    va_start (va, format);
    vfprintf (stderr, format, va);
    va_end (va);
    if (colorize_errors) {
        fprintf (stderr, "\e[0m");
    }
    fprintf (stderr, "\n");
}
