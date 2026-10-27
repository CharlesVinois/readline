#ifndef COUNT_H
#define COUNT_H

#include <stdio.h>

struct Counts {
    unsigned long long lines;
    unsigned long long words;
    unsigned long long bytes;
    unsigned long long characters;
};

enum CountStatus {
    COUNT_STATUS_OK,
    COUNT_STATUS_READ_ERROR,
    COUNT_STATUS_INVALID_UTF8,
    COUNT_STATUS_INCOMPLETE_UTF8
};

enum CountStatus count_stream(FILE *file, int count_characters,
                              struct Counts *counts);

#endif
