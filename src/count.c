#include "readline_stats.h"

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

enum readline_status readline_count_stream(
    FILE *stream,
    unsigned int options,
    struct readline_counts *counts)
{
    if (stream == NULL || counts == NULL || options == 0
        || (options & ~READLINE_COUNT_ALL) != 0) {
        return READLINE_STATUS_INVALID_ARGUMENT;
    }

    struct readline_counts result = {0};
    unsigned int utf8_remaining = 0;
    unsigned char utf8_next_min = 0x80;
    unsigned char utf8_next_max = 0xBF;
    int in_word = 0;
    int last_byte = '\n';
    int byte;

    while ((byte = fgetc(stream)) != EOF) {
        unsigned char value = (unsigned char)byte;

        if ((options & READLINE_COUNT_BYTES) != 0) {
            result.bytes++;
        }
        if ((options & READLINE_COUNT_LINES) != 0 && value == '\n') {
            result.lines++;
        }
        if ((options & READLINE_COUNT_WORDS) != 0) {
            if (isspace(value)) {
                in_word = 0;
            } else if (!in_word) {
                result.words++;
                in_word = 1;
            }
        }

        if ((options & READLINE_COUNT_CHARACTERS) != 0) {
            int starts_character = (utf8_remaining == 0);
            if (!update_utf8_state(
                    value, &utf8_remaining, &utf8_next_min, &utf8_next_max)) {
                return READLINE_STATUS_INVALID_UTF8;
            }
            if (starts_character) {
                result.characters++;
            }
        }

        last_byte = value;
    }

    if (ferror(stream)) {
        return READLINE_STATUS_READ_ERROR;
    }
    if ((options & READLINE_COUNT_CHARACTERS) != 0 && utf8_remaining != 0) {
        return READLINE_STATUS_INCOMPLETE_UTF8;
    }
    if ((options & READLINE_COUNT_LINES) != 0 && last_byte != '\n') {
        result.lines++;
    }

    *counts = result;
    return READLINE_STATUS_OK;
}
