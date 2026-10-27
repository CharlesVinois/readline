#include "readline_stats.h"

#include <stdio.h>
#include <string.h>

struct Options {
    int lines;
    int words;
    int bytes;
    int characters;
    int help;
};

static void print_usage(const char *program)
{
    printf("Utilisation : %s [OPTIONS] <fichier>\n", program);
    puts("Compte différentes caractéristiques du fichier.");
    puts("Sans option, affiche cette aide.");
    puts("");
    puts("Options :");
    puts("  -l, --lines       compter les lignes");
    puts("  -w, --words       compter les mots");
    puts("  -c, --bytes       compter les octets");
    puts("  -m, --characters  compter les caractères UTF-8");
    puts("  -h, --help        afficher cette aide");
    puts("");
    puts("Les options de comptage peuvent être combinées.");
}

static int enable_option(struct Options *options, char option)
{
    switch (option) {
    case 'l':
        options->lines = 1;
        return 1;
    case 'w':
        options->words = 1;
        return 1;
    case 'c':
        options->bytes = 1;
        return 1;
    case 'm':
        options->characters = 1;
        return 1;
    case 'h':
        options->help = 1;
        return 1;
    default:
        return 0;
    }
}

int main(int argc, char *argv[])
{
    struct Options options = {0, 0, 0, 0, 0};
    const char *filename = NULL;
    int parse_options = 1;

    for (int i = 1; i < argc; i++) {
        const char *argument = argv[i];

        if (parse_options && strcmp(argument, "--") == 0) {
            parse_options = 0;
        } else if (parse_options && argument[0] == '-'
                   && argument[1] != '\0') {
            if (argument[1] == '-') {
                if (strcmp(argument, "--lines") == 0) {
                    options.lines = 1;
                } else if (strcmp(argument, "--words") == 0) {
                    options.words = 1;
                } else if (strcmp(argument, "--bytes") == 0) {
                    options.bytes = 1;
                } else if (strcmp(argument, "--characters") == 0) {
                    options.characters = 1;
                } else if (strcmp(argument, "--help") == 0) {
                    options.help = 1;
                } else {
                    fprintf(stderr, "Option inconnue : %s\n", argument);
                    return 1;
                }
            } else {
                for (int j = 1; argument[j] != '\0'; j++) {
                    if (!enable_option(&options, argument[j])) {
                        fprintf(stderr, "Option inconnue : -%c\n", argument[j]);
                        return 1;
                    }
                }
            }
        } else if (filename == NULL) {
            filename = argument;
        } else {
            fprintf(stderr, "Un seul fichier peut être indiqué.\n");
            return 1;
        }
    }

    if (options.help || argc == 1) {
        print_usage(argv[0]);
        return 0;
    }
    if (!options.lines && !options.words && !options.bytes
        && !options.characters) {
        print_usage(argv[0]);
        return 0;
    }
    if (filename == NULL) {
        fprintf(stderr, "Indiquez un fichier à analyser.\n");
        print_usage(argv[0]);
        return 1;
    }

    FILE *file = fopen(filename, "rb");
    if (file == NULL) {
        perror(filename);
        return 1;
    }

    unsigned int count_options = 0;
    if (options.lines) {
        count_options |= READLINE_COUNT_LINES;
    }
    if (options.words) {
        count_options |= READLINE_COUNT_WORDS;
    }
    if (options.bytes) {
        count_options |= READLINE_COUNT_BYTES;
    }
    if (options.characters) {
        count_options |= READLINE_COUNT_CHARACTERS;
    }

    struct readline_counts counts;
    enum readline_status status =
        readline_count_stream(file, count_options, &counts);
    if (status == READLINE_STATUS_READ_ERROR) {
        perror(filename);
        fclose(file);
        return 1;
    }
    if (status == READLINE_STATUS_INVALID_UTF8) {
        fprintf(stderr, "%s : le fichier n'est pas un UTF-8 valide\n", filename);
        fclose(file);
        return 1;
    }
    if (status == READLINE_STATUS_INCOMPLETE_UTF8) {
        fprintf(stderr,
                "%s : le fichier se termine au milieu d'un caractère UTF-8\n",
                filename);
        fclose(file);
        return 1;
    }
    if (status != READLINE_STATUS_OK) {
        fprintf(stderr, "Impossible d'analyser le fichier : argument invalide\n");
        fclose(file);
        return 1;
    }
    if (fclose(file) == EOF) {
        perror(filename);
        return 1;
    }

    if (options.lines) {
        printf("Lignes : %llu\n", counts.lines);
    }
    if (options.words) {
        printf("Mots : %llu\n", counts.words);
    }
    if (options.bytes) {
        printf("Octets : %llu\n", counts.bytes);
    }
    if (options.characters) {
        printf("Caractères : %llu\n", counts.characters);
    }

    return 0;
}
