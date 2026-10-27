#include "count.h"

#include <ctype.h>

/* Valide un octet UTF-8 et suit les octets de continuation attendus. */
static int update_utf8_state(
    unsigned char byte,
    unsigned int *remaining,
    unsigned char *next_min,
    unsigned char *next_max)
{
    if (*remaining > 0) {
        if (byte < *next_min || byte > *next_max) {
            return 0;
        }
        (*remaining)--;
        *next_min = 0x80;
        *next_max = 0xBF;
        return 1;
    }

    if (byte <= 0x7F) {
        return 1;
    }
    if (byte >= 0xC2 && byte <= 0xDF) {
        *remaining = 1;
    } else if (byte >= 0xE0 && byte <= 0xEF) {
        *remaining = 2;
        if (byte == 0xE0) {
            *next_min = 0xA0;
        } else if (byte == 0xED) {
            *next_max = 0x9F;
        }
    } else if (byte >= 0xF0 && byte <= 0xF4) {
        *remaining = 3;
        if (byte == 0xF0) {
            *next_min = 0x90;
        } else if (byte == 0xF4) {
            *next_max = 0x8F;
        }
    } else {
        return 0;
    }

    return 1;
}

enum CountStatus count_stream(FILE *file, int count_characters,
                              struct Counts *counts)
{
    *counts = (struct Counts){0};
    unsigned int utf8_remaining = 0;
    unsigned char utf8_next_min = 0x80;
    unsigned char utf8_next_max = 0xBF;
    int in_word = 0;
    int last_byte = '\n';
    int byte;

    while ((byte = fgetc(file)) != EOF) {
        unsigned char value = (unsigned char)byte;

        counts->bytes++;
        if (value == '\n') {
            counts->lines++;
        }
        if (isspace(value)) {
            in_word = 0;
        } else if (!in_word) {
            counts->words++;
            in_word = 1;
        }

        if (count_characters) {
            int starts_character = (utf8_remaining == 0);
            if (!update_utf8_state(
                    value, &utf8_remaining, &utf8_next_min, &utf8_next_max)) {
                return COUNT_STATUS_INVALID_UTF8;
            }
            if (starts_character) {
                counts->characters++;
            }
        }

        last_byte = value;
    }

    if (ferror(file)) {
        return COUNT_STATUS_READ_ERROR;
    }
    if (count_characters && utf8_remaining != 0) {
        return COUNT_STATUS_INCOMPLETE_UTF8;
    }
    if (last_byte != '\n') {
        counts->lines++;
    }

    return COUNT_STATUS_OK;
}
