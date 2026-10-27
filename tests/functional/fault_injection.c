#include "readline_stats.h"

#include <errno.h>
#include <stdlib.h>

enum readline_status __real_readline_count_stream(
    FILE *stream,
    unsigned int options,
    struct readline_counts *counts);
int __real_fclose(FILE *stream);

enum readline_status __wrap_readline_count_stream(
    FILE *stream,
    unsigned int options,
    struct readline_counts *counts)
{
    if (getenv("READLINE_TEST_INVALID_ARGUMENT") != NULL) {
        return READLINE_STATUS_INVALID_ARGUMENT;
    }

    return __real_readline_count_stream(stream, options, counts);
}

int __wrap_fclose(FILE *stream)
{
    int status = __real_fclose(stream);

    if (getenv("READLINE_TEST_CLOSE_FAILURE") != NULL && status == 0) {
        errno = EIO;
        return EOF;
    }

    return status;
}
