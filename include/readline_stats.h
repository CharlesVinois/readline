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
 * Lit stream de sa position courante jusqu'à EOF. L'appelant doit fournir un
 * flux lisible ouvert en mode binaire ; cette fonction ne le rembobine pas et
 * ne le ferme pas. En cas de succès, la position du flux est à EOF.
 *
 * Les options peuvent être combinées, mais ne doivent être ni nulles ni
 * contenir de bits inconnus. En cas de succès, les compteurs demandés sont
 * renseignés et les autres valent zéro. En cas d'erreur, *counts reste
 * inchangé. stream et counts doivent être non NULL.
 *
 * Les lignes correspondent aux octets '\n' plus une dernière ligne non vide
 * qui ne se termine pas par '\n'. Les mots sont des séquences d'octets non
 * blancs, les espaces étant déterminés par isspace dans la locale courante.
 * Les caractères sont des points de code Unicode encodés en UTF-8 valide ;
 * les séquences combinées visuellement comptent donc plusieurs caractères.
 * Le comptage des octets mesure les octets lus.
 *
 * Renvoie READLINE_STATUS_INVALID_ARGUMENT pour des pointeurs invalides ou
 * des options invalides, READLINE_STATUS_READ_ERROR en cas d'erreur d'E/S,
 * READLINE_STATUS_INVALID_UTF8 pour un UTF-8 mal formé et
 * READLINE_STATUS_INCOMPLETE_UTF8 si le flux se termine au milieu d'un
 * caractère UTF-8. En cas d'erreur, le flux peut déjà avoir avancé.
 */
enum readline_status readline_count_stream(
    FILE *stream,
    unsigned int options,
    struct readline_counts *counts);

#endif
