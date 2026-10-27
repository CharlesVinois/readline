#include <readline_stats.h>

#include <stdio.h>

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Utilisation : %s <fichier>\n", argv[0]);
        return 1;
    }

    FILE *file = fopen(argv[1], "rb");
    if (file == NULL) {
        perror(argv[1]);
        return 1;
    }

    struct readline_counts counts;
    enum readline_status status = readline_count_stream(
        file, READLINE_COUNT_ALL, &counts);
    if (fclose(file) == EOF) {
        perror(argv[1]);
        return 1;
    }
    if (status != READLINE_STATUS_OK) {
        fprintf(stderr, "Échec du comptage (code %d)\n", status);
        return 1;
    }

    printf("Lignes : %llu\n", counts.lines);
    printf("Mots : %llu\n", counts.words);
    printf("Octets : %llu\n", counts.bytes);
    printf("Caractères : %llu\n", counts.characters);
    return 0;
}
