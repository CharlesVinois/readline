#ifndef READLINE_STATS_H
#define READLINE_STATS_H

#include <stdio.h>

enum readline_count_options {
    READLINE_COUNT_LINES = 1u << 0,
    READLINE_COUNT_WORDS = 1u << 1,
    READLINE_COUNT_BYTES = 1u << 2,
    READLINE_COUNT_CHARACTERS = 1u << 3,
    READLINE_COUNT_ALL = (1u << 4) - 1
};

enum readline_status {
    READLINE_STATUS_OK,
    READLINE_STATUS_INVALID_ARGUMENT,
    READLINE_STATUS_READ_ERROR,
    READLINE_STATUS_INVALID_UTF8,
    READLINE_STATUS_INCOMPLETE_UTF8
};

struct readline_counts {
    unsigned long long lines;
    unsigned long long words;
    unsigned long long bytes;
    unsigned long long characters;
};

/*
 * Compte les propriétés demandées d'un flux ouvert en mode binaire.
 * Le flux reste ouvert et appartient à l'appelant. Les résultats restent
 * inchangés en cas d'erreur. Le comptage des caractères valide l'UTF-8.
 *
 * Renvoie READLINE_STATUS_INVALID_ARGUMENT pour un pointeur NULL, un ensemble
 * d'options vide ou des bits inconnus ; READLINE_STATUS_READ_ERROR en cas
 * d'erreur d'E/S ; READLINE_STATUS_INVALID_UTF8 pour un UTF-8 mal formé ; et
 * READLINE_STATUS_INCOMPLETE_UTF8 si le dernier caractère est tronqué.
 */
enum readline_status readline_count_stream(
    FILE *stream,
    unsigned int options,
    struct readline_counts *counts);

#endif
